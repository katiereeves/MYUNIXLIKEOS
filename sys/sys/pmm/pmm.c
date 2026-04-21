#include "pmm.h"
#include "page.h"
#include "multiboot.h"
#include "stdio.h"
#include "stdbool.h"

static uint8_t pmm_bitmap[PMM_BITMAP_BYTES];

uint32_t pmm_end         = 0;
uint32_t pmm_total_pages = 0;

void bitmap_set(uint64_t bit) {
    if (bit >= pmm_total_pages)
        return;
    pmm_bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8));
}

void bitmap_clear(uint64_t bit) {
    if (bit >= pmm_total_pages)
        return;
    pmm_bitmap[bit / 8] &= (uint8_t)~(1u << (bit % 8));
}

int bitmap_test(uint64_t bit) {
    if (bit >= pmm_total_pages)
        return 1;
    return (pmm_bitmap[bit / 8] >> (bit % 8)) & 1;
}

static inline uint64_t addr_to_bit(uint64_t addr) {
    return (addr - PMM_START) / PAGE_SIZE;
}

void pmm_init(void *mmap_tag) {
    extern uint8_t _kernel_start[];
    extern uint8_t _kernel_end[];

    /* mark everything as used */
    for (size_t i = 0; i < sizeof(pmm_bitmap); i++)
        pmm_bitmap[i] = 0xFF;

    if (!mmap_tag) {
        /* no memory map from bootloader: fall back to 16MB */
        pmm_end         = 0x01000000;
        pmm_total_pages = (pmm_end - PMM_START) / PAGE_SIZE;

        for (uint32_t i = 0; i < pmm_total_pages; i++)
            pmm_bitmap[i / 8] &= (uint8_t)~(1u << (i % 8));
        goto mark_reserved;
    }

    {
        struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)mmap_tag;
        uint8_t *ep  = (uint8_t *)mmap->entries;
        uint8_t *end = (uint8_t *)mmap + mmap->size;
        uint64_t highest = 0;

        while (ep < end) {
            struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry *)ep;
            if (e->type == MULTIBOOT_MEMORY_AVAILABLE) {
                uint64_t top = e->addr + e->len;
                if (top > highest)
                    highest = top;
            }
            ep += mmap->entry_size;
        }

        if (highest > PMM_MAX_PHYS) highest = PMM_MAX_PHYS;
        if (highest <= PMM_START)   highest = PMM_START + PAGE_SIZE;

        pmm_end         = (uint32_t)highest;
        pmm_total_pages = (pmm_end - PMM_START) / PAGE_SIZE;
        if (pmm_total_pages > (uint32_t)PMM_MAX_PAGES)
            pmm_total_pages = (uint32_t)PMM_MAX_PAGES;

        ep = (uint8_t *)mmap->entries;

        while (ep < end) {
            struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry *)ep;

            if (e->type == MULTIBOOT_MEMORY_AVAILABLE) {
                uint64_t start = e->addr;
                uint64_t stop  = e->addr + e->len;

                if (start < PMM_START) start = PMM_START;
                if (stop  > pmm_end)   stop  = pmm_end;

                for (uint64_t addr = PAGE_ALIGN_UP(start);
                     addr + PAGE_SIZE <= stop;
                     addr += PAGE_SIZE) {
                    uint64_t bit = addr_to_bit(addr);
                    if (bit < pmm_total_pages)
                        pmm_bitmap[bit / 8] &= (uint8_t)~(1u << (bit % 8));
                }
            }
            ep += mmap->entry_size;
        }
    }

mark_reserved:
    uint64_t ks = (uint64_t)(uintptr_t)_kernel_start & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t ke = ((uint64_t)(uintptr_t)_kernel_end + PAGE_SIZE - 1)
                      & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t a = ks; a < ke; a += PAGE_SIZE) {
        if (a >= PMM_START && a < pmm_end) {
            uint64_t bit = addr_to_bit(a);
            if (bit < pmm_total_pages)
                pmm_bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8));
        }
    }

    uint64_t bs = (uint64_t)(uintptr_t)pmm_bitmap
                & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t be = ((uint64_t)(uintptr_t)pmm_bitmap + sizeof(pmm_bitmap) + PAGE_SIZE - 1)
                & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t a = bs; a < be; a += PAGE_SIZE) {
        if (a >= PMM_START && a < pmm_end) {
            uint64_t bit = addr_to_bit(a);
            if (bit < pmm_total_pages)
                pmm_bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8));
        }
    }
}

void *pmm_alloc_page(void) {
    for (uint32_t i = 0; i < pmm_total_pages; i++) {
        if (!bitmap_test(i)) {
            pmm_bitmap[i / 8] |= (uint8_t)(1u << (i % 8));
            return (void *)(uintptr_t)(PMM_START + (uint64_t)i * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmm_free_page(void *ptr) {
    uint64_t addr = (uint64_t)(uintptr_t)ptr;
    if (addr < PMM_START || addr >= pmm_end)
        return;
    if (!PAGE_IS_ALIGNED(addr))
        return;
    uint64_t bit = addr_to_bit(addr);
    if (bit < pmm_total_pages)
        pmm_bitmap[bit / 8] &= (uint8_t)~(1u << (bit % 8));
}

void *pmm_alloc_blocks(size_t count) {
    if (count == 0)
        return NULL;

    for (uint32_t i = 0; i + count <= pmm_total_pages; i++) {
        bool found = true;
        for (size_t j = 0; j < count; j++) {
            if (bitmap_test(i + j)) {
                found = false;
                break;
            }
        }

        if (found) {
            for (size_t j = 0; j < count; j++)
                pmm_bitmap[(i + j) / 8] |= (uint8_t)(1u << ((i + j) % 8));
            return (void *)(uintptr_t)(PMM_START + (uint64_t)i * PAGE_SIZE);
        }
    }
    return NULL;
}

void *pmm_alloc_z(size_t size) {
    void *p = pmm_alloc_page();
    if (!p)
        return NULL;
    size_t n = size < PAGE_SIZE ? size : PAGE_SIZE;
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)p)[i] = 0;
    return p;
}