#include "page.h"
#include "pmm.h"
#include "string.h"
#include "stdio.h"

#define KERNEL_MAX_TABLES 32

static page_dir_entry_t   kernel_dir[1024]
    __attribute__((aligned(4096)));
static page_table_entry_t kernel_tables[KERNEL_MAX_TABLES][1024]
    __attribute__((aligned(4096)));

extern void paging_enable(uint32_t page_dir_phys);

void map_page(page_dir_entry_t *dir, uint32_t vaddr, uint32_t paddr,
              uint32_t size, uint32_t flags) {
    for (uint32_t off = 0; off < size; off += PAGE_SIZE) {
        uint32_t va  = vaddr + off;
        uint32_t pa  = paddr + off;
        uint32_t pdi = PAGE_DIR_INDEX(va);
        uint32_t pti = PAGE_TABLE_INDEX(va);

        page_dir_entry_t *kpd = get_kernel_page_dir();
        int is_user_region = (va < pmm_end);

        if (!page_dir_entry_present(dir[pdi])) {
            if (!is_user_region || dir[pdi] == kpd[pdi])
                dir[pdi] = kpd[pdi];
            else {
                page_table_entry_t *table = pmm_alloc_page();
                if (!table) { printf("map_page: OOM\n"); return; }
                memset(table, 0, PAGE_SIZE);
                dir[pdi] = page_dir_entry_make((uint32_t)table,
                    PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
            }
        }
        else if (is_user_region &&
                dir[pdi] == kpd[pdi] &&
                page_dir_entry_present(dir[pdi])) {
            page_table_entry_t *old =
                (page_table_entry_t *)page_dir_entry_addr(dir[pdi]);
            page_table_entry_t *table = pmm_alloc_page();

            if (!table) {
                printf("map_page: OOM\n");
                return;
            }
            memcpy(table, old, PAGE_SIZE);
            dir[pdi] = page_dir_entry_make((uint32_t)table,
                PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
        }
        else if (flags & PAGE_USER)
            dir[pdi] |= PAGE_USER;

        page_table_entry_t *table =
            (page_table_entry_t *)page_dir_entry_addr(dir[pdi]);
        table[pti] = page_table_entry_make(pa, flags);
        page_tlb_flush(va);
    }
}

void unmap_page(page_dir_entry_t *dir, uint32_t vaddr, uint32_t size) {
    for (uint32_t off = 0; off < size; off += PAGE_SIZE) {
        uint32_t va  = vaddr + off;
        uint32_t pdi = PAGE_DIR_INDEX(va);
        uint32_t pti = PAGE_TABLE_INDEX(va);

        if (!page_dir_entry_present(dir[pdi]))
            continue;

        page_table_entry_t *table =
            (page_table_entry_t *)page_dir_entry_addr(dir[pdi]);
        table[pti] = 0;
        page_tlb_flush(va);
    }
}

void page_init(void) {
    memset(kernel_dir, 0, sizeof(kernel_dir));

    uint32_t num_tables = (pmm_end + (4*1024*1024) - 1) / (4*1024*1024);
    if (num_tables > KERNEL_MAX_TABLES) num_tables = KERNEL_MAX_TABLES;

    for (uint32_t t = 0; t < num_tables; t++) {
        memset(kernel_tables[t], 0, PAGE_SIZE);
        for (int pg = 0; pg < 1024; pg++) {
            uint32_t phys = (t * 1024 + pg) * PAGE_SIZE;
            kernel_tables[t][pg] = page_table_entry_make(phys,
                PAGE_PRESENT | PAGE_WRITE);
        }
        kernel_dir[t] = page_dir_entry_make((uint32_t)kernel_tables[t],
            PAGE_PRESENT | PAGE_WRITE);
    }

    paging_enable((uint32_t)kernel_dir);
    printf("mem = %iMB\n", num_tables * 4);
}

uint32_t *clone_page_directory(uint32_t *parent_pd) {
    uint32_t *child_pd = pmm_alloc_page();
    if (!child_pd) return NULL;
    memset(child_pd, 0, PAGE_SIZE);

    uint32_t *kpd = get_kernel_page_dir();

    for (int i = 0; i < 1024; i++) {
        if (parent_pd[i] == kpd[i]) {
            child_pd[i] = parent_pd[i];
            continue;
        }
        if (!(parent_pd[i] & PAGE_PRESENT))
            continue;

        uint32_t *ppt = (uint32_t *)(parent_pd[i] & ~0xFFFu);
        uint32_t *cpt = pmm_alloc_page();
        if (!cpt)
            return NULL;
        memset(cpt, 0, PAGE_SIZE);

        for (int j = 0; j < 1024; j++) {
            if (!(ppt[j] & PAGE_PRESENT))
                continue;

            uint32_t old_phys = ppt[j] & ~0xFFFu;
            uint32_t new_phys = (uint32_t)pmm_alloc_page();
            if (!new_phys)
                return NULL;
            memcpy((void *)new_phys, (void *)old_phys, PAGE_SIZE);
            cpt[j] = new_phys | (ppt[j] & 0xFFFu);
        }
        child_pd[i] = ((uint32_t)cpt) | (parent_pd[i] & 0xFFFu);
    }
    return child_pd;
}

void switch_page_dir(uint32_t page_dir_phys) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(page_dir_phys) : "memory");
}

page_dir_entry_t *get_kernel_page_dir(void) {
    return kernel_dir;
}