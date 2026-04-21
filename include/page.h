#ifndef _PAGE_H_
#define _PAGE_H_

#include "stdint.h"

#define PAGE_SIZE     0x1000
#define PAGE_SHIFT    12
#define PAGE_MASK     (~(PAGE_SIZE - 1))

#define PMM_START        0x00100000UL    /* skip BIOS, VGA... */
#define PMM_MAX_PHYS     0x100000000ULL  /* 4GB */
#define PMM_MAX_PAGES    ((PMM_MAX_PHYS - PMM_START) / PAGE_SIZE)
#define PMM_BITMAP_BYTES (PMM_MAX_PAGES / 8) /* 128 KB */

extern uint32_t pmm_end;
extern uint32_t pmm_total_pages;

#define PAGE_PRESENT   (1u << 0)
#define PAGE_WRITE     (1u << 1)
#define PAGE_USER      (1u << 2)
#define PAGE_PWT       (1u << 3)
#define PAGE_PCD       (1u << 4)
#define PAGE_ACCESSED  (1u << 5)
#define PAGE_DIRTY     (1u << 6)
#define PAGE_HUGE      (1u << 7)
#define PAGE_GLOBAL    (1u << 8)

#define PAGE_FRAME_ADDR(entry)   ((entry) & PAGE_MASK)
#define PAGE_DIR_INDEX(vaddr)    ((vaddr) >> 22)
#define PAGE_TABLE_INDEX(vaddr)  (((vaddr) >> PAGE_SHIFT) & 0x3FF)
#define PAGE_ALIGN_UP(addr)      (((addr) + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_ALIGN_DOWN(addr)    ((addr) & PAGE_MASK)
#define PAGE_IS_ALIGNED(addr)    (((addr) & ~PAGE_MASK) == 0)

typedef uint32_t page_dir_entry_t;
typedef uint32_t page_table_entry_t;

#define page_dir_entry_make(table_phys, flags) \
    (PAGE_FRAME_ADDR(table_phys) | ((flags) & 0xFFF))

#define page_table_entry_make(page_phys, flags) \
    (PAGE_FRAME_ADDR(page_phys) | ((flags) & 0xFFF))

#define page_dir_entry_addr(entry)   PAGE_FRAME_ADDR(entry)
#define page_table_entry_addr(entry) PAGE_FRAME_ADDR(entry)

#define page_dir_entry_present(e)    ((e) & PAGE_PRESENT)
#define page_table_entry_present(e)  ((e) & PAGE_PRESENT)
#define page_table_entry_dirty(e)    ((e) & PAGE_DIRTY)
#define page_table_entry_user(e)     ((e) & PAGE_USER)
#define page_table_entry_write(e)    ((e) & PAGE_WRITE)

#define page_tlb_flush(vaddr) \
    __asm__ volatile("invlpg (%0)" : : "r"((uint32_t)(vaddr)) : "memory")

#define page_tlb_flush_all()            \
    do {                                \
        uint32_t _cr3;                  \
        __asm__ volatile(               \
            "mov %%cr3, %0\n"           \
            "mov %0,   %%cr3\n"         \
            : "=r"(_cr3) : : "memory"   \
        );                              \
    } while (0)

void              page_init(void);
void              switch_page_dir(uint32_t page_dir_phys);
page_dir_entry_t *get_kernel_page_dir(void);
void              map_page(page_dir_entry_t *dir, uint32_t vaddr,
                           uint32_t paddr, uint32_t size, uint32_t flags);
void              unmap_page(page_dir_entry_t *dir, uint32_t vaddr,
                             uint32_t size);
uint32_t         *clone_page_directory(uint32_t *parent_pd);

#endif /* page.h */