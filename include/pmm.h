#ifndef _PMM_H_
#define _PMM_H_

#include "stdint.h"
#include "stddef.h"

void  pmm_init(void *mmap_tag);
void* pmm_alloc_page();             /* Find one free 4KB page */
void pmm_free_page(void *ptr);
void* pmm_alloc_blocks(size_t count);
void* pmm_alloc_z(size_t size);     /* Allocate zeroed pages */

/* Internal bit manipulation */
void bitmap_set(uint64_t bit);
void bitmap_clear(uint64_t bit);
int  bitmap_test(uint64_t bit);

#define PMM_START      0x00100000UL   /* 1MB */
#define PMM_MAX_PHYS   0x100000000ULL /* 4GB — bitmap sized for this */
#define PMM_MAX_PAGES  ((PMM_MAX_PHYS - PMM_START) / PAGE_SIZE) /* ~1M pages */
#define PMM_BITMAP_SIZE (PMM_MAX_PAGES / 8)  /* 128KB */

#endif 