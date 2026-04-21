#include "proc.h"
#include "page.h"
#include "pmm.h"
#include "elf.h"
#include "stddef.h"
#include "string.h"
#include "vfs.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdio.h"
#include "gdt.h"

struct proc *proc_from_elf(const char *path) {
    if (sizeof(struct proc) > PAGE_SIZE) {
        printf("FATAL: sizeof(struct proc)=%u > PAGE_SIZE\n", (uint32_t)sizeof(struct proc));
        __asm__ volatile("cli; hlt");
    }

    struct proc *p = pmm_alloc_page();
    if (!p) return NULL;
    memset(p, 0, sizeof(struct proc));
    p->child_head = NULL; 
    p->next_child = NULL;
    p->parent = NULL;

    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0) {
        printf("proc_from_elf: can't open %s\n", path);
        pmm_free_page(p);
        return NULL;
    }

    Elf32_Ehdr ehdr;
    vfs_read(fd, &ehdr, sizeof(ehdr));

    if (ehdr.e_ident[0] != 0x7f || ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L'  || ehdr.e_ident[3] != 'F') {
        printf("proc_from_elf: bad magic\n");
        vfs_close(fd);
        pmm_free_page(p);
        return NULL;
    }

    Elf32_Phdr phdrs[ehdr.e_phnum];
    for (int i = 0; i < ehdr.e_phnum; i++) {
        vfs_lseek(fd, ehdr.e_phoff + i * sizeof(Elf32_Phdr), SEEK_SET);
        vfs_read(fd, &phdrs[i], sizeof(Elf32_Phdr));
    }

    uint32_t *pd = pmm_alloc_page();
    if (!pd) { vfs_close(fd); pmm_free_page(p); return NULL; }
    memset(pd, 0, PAGE_SIZE);

    uint32_t *kpd = get_kernel_page_dir();
    for (int i = 0; i < 1024; i++)
        pd[i] = kpd[i];

    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdrs[i].p_type != PT_LOAD) continue;

        uint32_t memsz  = phdrs[i].p_memsz;
        uint32_t filesz = phdrs[i].p_filesz;
        uint32_t vaddr  = phdrs[i].p_vaddr;
        uint32_t npages = (memsz + PAGE_SIZE - 1) / PAGE_SIZE;

        uint8_t *phys = pmm_alloc_blocks(npages);
        if (!phys) {
            printf("proc_from_elf: OOM segment %d\n", i);
            vfs_close(fd);
            return NULL;
        }

        memset(phys, 0, npages * PAGE_SIZE);
        vfs_lseek(fd, phdrs[i].p_offset, SEEK_SET);
        vfs_read(fd, phys, filesz);

        map_page(pd, vaddr, (uint32_t)phys,
                 npages * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

        uint32_t seg_end = vaddr + memsz;
        if (seg_end > p->p_brk) p->p_brk = seg_end;
    }

    vfs_close(fd);

    p->p_brk     = (p->p_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    p->p_brk_max = p->p_brk + PAGE_SIZE * 8;

    uint32_t stack_pages = 8;
    uint32_t stack_phys  = (uint32_t)pmm_alloc_blocks(stack_pages);
    uint32_t stack_virt  = 0x1FF000 - (stack_pages * PAGE_SIZE);

    map_page(pd, stack_virt, stack_phys,
            stack_pages * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    uint32_t path_len = strlen(path) + 1;
    uint32_t str_phys = stack_phys + (stack_pages * PAGE_SIZE) - path_len;
    memcpy((void*)str_phys, path, path_len);

    uint32_t str_virt = 0x1FF000 - path_len;
    
    uint32_t frame_phys = str_phys & ~3u;
    uint32_t *frame = (uint32_t *)frame_phys;

    *--frame = 0;         /* argv[1] = NULL */
    *--frame = str_virt;  /* argv[0] = path */
    *--frame = 1;         /* argc = 1 */

    map_page(pd, stack_virt, stack_phys,
            stack_pages * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    ((uint32_t *)(stack_phys + stack_pages * PAGE_SIZE - 8))[0] = 0; /* argc */
    ((uint32_t *)(stack_phys + stack_pages * PAGE_SIZE - 8))[1] = 0; /* argv */

    ((uint32_t *)(stack_phys + 0xFF8))[0] = 0;   /* argc = 0    */
    ((uint32_t *)(stack_phys + 0xFF8))[1] = 0;   /* argv = NULL */

    uint32_t kstack_phys = (uint32_t)pmm_alloc_page();
    if (!kstack_phys) {
        printf("proc_from_elf: OOM kernel stack\n");
        return NULL;
    }
    p->p_kernel_stack = kstack_phys + PAGE_SIZE;

    memset(&p->p_ctx, 0, sizeof(p->p_ctx));
    p->p_ctx.eip     = ehdr.e_entry;
    p->p_stack_base = stack_virt + ((uint32_t)frame - stack_phys);
    p->p_ctx.useresp = p->p_stack_base;
    p->p_ctx.cs      = 0x1B;
    p->p_ctx.ss      = 0x23;
    p->p_ctx.ds      = 0x23;
    p->p_ctx.es      = 0x23;
    p->p_ctx.fs      = 0x23;
    p->p_ctx.gs      = 0x23;
    p->p_ctx.eflags  = 0x202;

    p->p_page_dir = (uint32_t)pd;
    p->p_state    = PROC_READY;

    vfs_close(fd);

    return p;
}

void free_proc(struct proc *p) {
    if (!p)
        return;

    if (p->p_kernel_stack)
        pmm_free_page((void*)(p->p_kernel_stack - PAGE_SIZE));

    uint32_t *page_dir = (uint32_t *)p->p_page_dir;
    uint32_t *kpd      = get_kernel_page_dir();
    for (int i = 0; i < 1024; i++) {
        if (!(page_dir[i] & PAGE_PRESENT))
            continue;
        if (page_dir[i] == kpd[i])
            continue;
        uint32_t *pt = (uint32_t *)(page_dir[i] & ~0xFFFu);
        for (int j = 0; j < 1024; j++)
            if ((pt[j] & PAGE_PRESENT) && (pt[j] & PAGE_USER))
                pmm_free_page((void*)(uintptr_t)(pt[j] & ~0xFFFu));
        pmm_free_page(pt);
    }
    pmm_free_page(page_dir);
    pmm_free_page(p);
}

extern void jump_usermode(uint32_t entry, uint32_t user_stack);

void proc_run_once(const char *path) {
    struct proc *p = proc_from_elf(path);
    if (!p) {
        printf("proc_run_once: failed to load %s\n", path);
        return;
    }
    current_proc = p;
    tss_set_kernel_stack(p->p_kernel_stack);
    switch_page_dir(p->p_page_dir);
    jump_usermode(p->p_ctx.eip, p->p_stack_base);
}