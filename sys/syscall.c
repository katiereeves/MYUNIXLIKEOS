/* TODO: possibly move table functions out of here soon... */
#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "sys/syscall.h"
#include "fcntl.h"
#include "vfs.h"
#include "nvram.h"
#include "sys/time.h"
#include "elf.h"
#include "io.h"
#include "unistd.h"
#include "page.h"
#include "pmm.h"
#include "sys/proc/proc.h"
#include "sys/sched/sched.h"
#include "gdt.h"

#define SYSCALL_TABLE_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

typedef void (*syscall_fn)(struct trapframe *);

static void proc_build_resume_frame(struct proc *p) {
    struct trapframe *ctx = &p->p_ctx;
    uint32_t *ksp = (uint32_t *)p->p_kernel_stack;

    *--ksp = ctx->ss;
    *--ksp = ctx->useresp;
    *--ksp = ctx->eflags | 0x200;
    *--ksp = ctx->cs;
    *--ksp = ctx->eip;
    *--ksp = 0;
    *--ksp = 0x20;
    *--ksp = ctx->eax;
    *--ksp = ctx->ecx;
    *--ksp = ctx->edx;
    *--ksp = ctx->ebx;
    *--ksp = ctx->useresp;
    *--ksp = ctx->ebp;
    *--ksp = ctx->esi;
    *--ksp = ctx->edi;
    *--ksp = ctx->ds;
    *--ksp = ctx->es;
    *--ksp = ctx->fs;
    *--ksp = ctx->gs;

    p->p_ksp = (uint32_t)ksp;
}

static void voluntary_switch(struct trapframe *regs) {
    tss_set_kernel_stack(current_proc->p_kernel_stack);
    switch_page_dir(current_proc->p_page_dir);
    current_proc->p_state = PROC_RUNNING;

    *regs = current_proc->p_ctx;
    regs->eflags |= 0x200;
}

static void pick_next(struct trapframe *regs) {
    if (!run_queue_head) {
        struct proc *sh = proc_from_elf("sh");
        if (!sh) {
            printf("shell failted to start: halting\n");
            __asm__ volatile("cli; hlt");
        }
        sched_enqueue(sh);
    }

    struct proc *p    = run_queue_head;
    struct proc *stop = p;
    do {
        if (p->p_state == PROC_READY) {
            run_queue_head = p;
            current_proc   = p;
            voluntary_switch(regs);
            return;
        }
        p = p->next;
    } while (p != stop);
}

static void sys_sched_yield(struct trapframe *regs) {
    if (!run_queue_head) {
        regs->eax = (uint32_t)-1;
        return;
    }

    current_proc->p_ctx   = *regs;
    current_proc->p_ksp   = (uint32_t)regs;
    current_proc->p_state = PROC_READY;

    run_queue_head = run_queue_head->next;

    pick_next(regs);
    regs->eax = 0;
}

static void sys_mkdir(struct trapframe *regs) {
    vfs_mkdir((const char *)regs->ebx);
}

static void sys_open(struct trapframe *regs){
    const char *path = (const char *)regs->ebx;
    int flags = (int)regs->ecx;

    if (flags & O_CREAT)
        if (!vfs_lookup(path))
            vfs_create(path);

    regs->eax = (uint32_t)vfs_open(path, flags);
}

static void sys_close(struct trapframe *regs) {
    int fd = (int)regs->ebx;
    if (fd < 0 || fd >= VFS_MAX_FDS) {
        regs->eax = (uint32_t)-1;
        return;
    }
    regs->eax = (uint32_t)vfs_close(fd);
}

static void sys_read(struct trapframe *regs) {
    int fd         = (int)regs->ebx;
    void *buf      = (void *)regs->ecx;
    uint32_t count = regs->edx;

    if (fd < 0 || fd >= VFS_MAX_FDS) {
        regs->eax = (uint32_t)-1;
        return;
    }

    if (fd == 0) {
        char *b = (char *)buf;
        for (uint32_t i = 0; i < count; i++)
            b[i] = keyboard_getchar();
        regs->eax = count;
        return;
    }

    regs->eax = (uint32_t)vfs_read(fd, buf, count);
}

static void sys_getdents(struct trapframe *regs){
    regs->eax = (uint32_t)vfs_getdents(
        (int)regs->ebx,
        (void *)regs->ecx,
        regs->edx
    );
}

static void sys_lseek(struct trapframe *regs) {
    regs->eax = (uint32_t)vfs_lseek(
        (int)regs->ebx,
        (long)regs->ecx,
        (int)regs->edx
    );
}

static void handle_ansi(const char *buf, uint32_t count, uint32_t *ip){
    uint32_t i = *ip + 2; /* skip esc [ */

    int params[2] = {0, 0};
    int nparam    = 0;
    while (i < count) {
        char d = buf[i];
        if (d >= '0' && d <= '9') {
            params[nparam] = params[nparam] * 10 + (d - '0');
            i++;
        } else if (d == ';') {
            nparam = (nparam + 1) % 2;
            i++;
        } else
            break;
    }

    if (i >= count) {
        *ip = i;
        return;
    }
    char cmd = buf[i];

    switch (cmd) {
        case 'H': case 'f':
            move_cursor(
                params[1] > 0 ? params[1] - 1 : 0,
                params[0] > 0 ? params[0] - 1 : 0
            );
            break;
        case 'A':
            move_cursor((int)terminal_col, (int)terminal_row - (params[0] ? params[0] : 1));
            break;
        case 'B':
            move_cursor((int)terminal_col, (int)terminal_row + (params[0] ? params[0] : 1));
            break;
        case 'C':
            move_cursor((int)terminal_col + (params[0] ? params[0] : 1), (int)terminal_row);
            break;
        case 'D':
            move_cursor((int)terminal_col - (params[0] ? params[0] : 1), (int)terminal_row);
            break;
        case 'J':
            if (params[0] == 2) {
                for (size_t col = 0; col < 80; col++)
                    for (size_t row = 0; row < 25; row++) {
                        move_cursor(col, row);
                        terminal_putc(' ');
                    }
                move_cursor(0, 0);
            }
            break;
        case 'K': {
            int saved_col = (int)terminal_col;
            int saved_row = (int)terminal_row;
            terminal_clear_line(saved_row);
            move_cursor(saved_col, saved_row);
            break;
        }
        default:
            break;
    }

    *ip = i;
}

static void sys_write(struct trapframe *regs){
    int fd          = (int)regs->ebx;
    const char *buf = (const char *)regs->ecx;
    uint32_t count  = regs->edx;

    if ((fd == 1 || fd == 2) && buf && count) {
        for (uint32_t i = 0; i < count; i++) {
            char c = buf[i];
            if (c == '\033' && i+1 < count && buf[i+1] == '[')
                handle_ansi(buf, count, &i);
            else if (c == '\b' || c == 0x7F)
                terminal_backspace();
            else
                terminal_putc(c);
        }
        regs->eax = count;
    }
    else if (fd > 2 && buf && count)
        regs->eax = (uint32_t)vfs_write(fd, buf, count);
    else
        regs->eax = (uint32_t)-1;
}

#define IS_LEAP(y) (((y)%4==0&&(y)%100!=0)||((y)%400==0))

static void sys_clock_gettime(struct trapframe *regs){
    if (regs->ebx != CLOCK_REALTIME) {
        regs->eax = (uint32_t)-1;
        return;
    }

    char nvbuf[128];
    struct nvram_t *nv = (struct nvram_t *)nvbuf;
    readNVRAM(nvbuf);

    struct timespec *tp = (struct timespec *)regs->ecx;

    int sec   = bcd(nv->rtc_sec);
    int min   = bcd(nv->rtc_min);
    int hour  = bcd(nv->rtc_hour);
    int day   = bcd(nv->rtc_day);
    int month = bcd(nv->rtc_month);
    int year  = bcd(nv->century_BCD) * 100 + bcd(nv->rtc_year);

    uint8_t days_per_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    time_t total_days = 0;
    for (int y = 1970; y < year; y++)
        total_days += IS_LEAP(y) ? 366 : 365;

    for (int m = 0; m < month - 1; m++) {
        total_days += days_per_month[m];
        if (m == 1 && IS_LEAP(year))
            total_days += 1;
    }

    total_days += day - 1;
    tp->tv_sec  = total_days * 86400 + hour * 3600 + min * 60 + sec;
    tp->tv_nsec = 0;
    regs->eax   = 0;
}

static void sys_execl(struct trapframe *regs) {
    const char  *path = (const char  *)regs->ebx;
    const char **argv = (const char **)regs->ecx;

    char     kbuf[1024];
    uint32_t koff[16];
    int      argc = 0;
    char    *kp   = kbuf;

    if (argv) {
        while (argc < 15 && argv[argc]) {
            size_t len = strlen(argv[argc]) + 1;
            if (kp + len > kbuf + sizeof(kbuf)) break;
            memcpy(kp, argv[argc], len);
            koff[argc] = (uint32_t)(kp - kbuf);
            kp += len; argc++;
        }
    }

    struct proc *newp = proc_from_elf(path);
    if (!newp) {
        printf("exec: failed to load '%s'\n", path);
        regs->eax=(uint32_t)-1;
        return;
    }

    uint32_t new_pd   = newp->p_page_dir,  new_eip  = newp->p_ctx.eip;
    uint32_t new_sb   = newp->p_stack_base,new_brk  = newp->p_brk;
    uint32_t new_brkx = newp->p_brk_max,   new_kst  = newp->p_kernel_stack;
    pmm_free_page(newp);

    uint32_t *old_pd  = (uint32_t *)current_proc->p_page_dir;
    uint32_t *kpd_ptr = get_kernel_page_dir();
    for (int i = 0; i < 1024; i++) {
        if (!(old_pd[i] & PAGE_PRESENT) || old_pd[i] == kpd_ptr[i])
            continue;
        uint32_t *pt = (uint32_t *)(old_pd[i] & ~0xFFFu);
        for (int j = 0; j < 1024; j++)
            if ((pt[j] & PAGE_PRESENT) && (pt[j] & PAGE_USER))
                pmm_free_page((void*)(uintptr_t)(pt[j] & ~0xFFFu));
        pmm_free_page(pt);
    }
    pmm_free_page(old_pd);
    if (current_proc->p_kernel_stack)
        pmm_free_page((void*)(current_proc->p_kernel_stack - PAGE_SIZE));

    current_proc->p_page_dir     = new_pd;
    current_proc->p_brk          = new_brk;
    current_proc->p_brk_max      = new_brkx;
    current_proc->p_stack_base   = new_sb;
    current_proc->p_kernel_stack = new_kst;

    tss_set_kernel_stack(new_kst);
    switch_page_dir(new_pd);

    uint8_t *strptr = (uint8_t *)new_sb;
    uint32_t user_ptrs[16];
    for (int i = argc-1; i >= 0; i--) {
        size_t len = strlen(kbuf+koff[i])+1;
        strptr -= len;
        memcpy(strptr, kbuf+koff[i], len);
        user_ptrs[i] = (uint32_t)strptr;
    }
    strptr = (uint8_t *)((uint32_t)strptr & ~3u);
    uint32_t *frame = (uint32_t *)strptr;
    frame--; *frame = 0;
    for (int i = argc-1; i >= 0; i--) {
        frame--;
        *frame = user_ptrs[i];
    }
    frame--; *frame = (uint32_t)argc;

    regs->eip     = new_eip;
    regs->useresp = (uint32_t)frame;
    regs->cs      = 0x1B;
    regs->ss      = 0x23;
    regs->ds      = 0x23;
    regs->es      = 0x23;
    regs->fs      = 0x23;
    regs->gs      = 0x23;
    regs->eflags  = 0x202;
    regs->eax = regs->ebx = regs->ecx = 0;
    regs->edx = regs->esi = regs->edi = regs->ebp = 0;
}

static void proc_free_address_space(struct proc *p) {
    if (!p->p_page_dir)
        return;
 
    uint32_t *pd  = (uint32_t *)p->p_page_dir;
    uint32_t *kpd = get_kernel_page_dir();
 
    for (int i = 0; i < 1024; i++) {
        if (!(pd[i] & PAGE_PRESENT))
            continue;
        if (pd[i] == kpd[i])
            continue;
 
        uint32_t *pt = (uint32_t *)(pd[i] & ~0xFFFu);
        for (int j = 0; j < 1024; j++)
            if ((pt[j] & PAGE_PRESENT) && (pt[j] & PAGE_USER))
                pmm_free_page((void *)(uintptr_t)(pt[j] & ~0xFFFu));

        pmm_free_page(pt);
    }
 
    pmm_free_page(pd);
    p->p_page_dir = 0;
}

static void sys_exit(struct trapframe *regs) {
    struct proc *p = current_proc;
    p->p_exitcode  = (int)regs->ebx;
 
    sched_dequeue(p);
 
    struct proc *ch = p->child_head;
    while (ch) {
        struct proc *nx = ch->next_child;
        if (ch->p_state == PROC_ZOMBIE) {
            proc_free_address_space(ch);
            sched_defer_free(ch);
        } else
            ch->parent = NULL;
        ch = nx;
    }
    p->child_head = NULL;

    struct proc *par = p->parent;
    bool has_living_parent = par &&
        par->p_state != PROC_DEAD &&
        par->p_state != PROC_ZOMBIE;
 
    if (has_living_parent) {
        p->p_state = PROC_ZOMBIE;

        if (par->p_state == PROC_WAITING) {
            par->p_ctx.eax = (uint32_t)p->p_id;
            proc_build_resume_frame(par);
            sched_enqueue(par);
        }

        pick_next(regs);

        proc_free_address_space(p);
 
        if (p->p_kernel_stack) {
            pmm_free_page((void *)(p->p_kernel_stack - PAGE_SIZE));
            p->p_kernel_stack = 0;
        }
    }
    else {
        pick_next(regs);
        proc_free_address_space(p);
        sched_defer_free(p);
    }
}

static void sys_wait(struct trapframe *regs) {
    struct proc *p = current_proc;

    for (;;) {
        struct proc *zombie = NULL;
        struct proc *prev   = NULL;
        struct proc *curr   = p->child_head;

        for (; curr; curr = curr->next_child) {
            if (curr->p_state == PROC_ZOMBIE) {
                zombie = curr;
                break;
            }
            prev = curr;
            
        }

        if (zombie) {
            regs->eax = (uint32_t)zombie->p_id;

            if (prev) prev->next_child = zombie->next_child;
            else p->child_head = zombie->next_child;

            sched_defer_free(zombie);
            return;
        }

        p->p_state = PROC_WAITING;
        p->p_ctx   = *regs; 
        proc_build_resume_frame(p);
        pick_next(regs);
    }
}

static void sys_sbrk(struct trapframe *regs) {
    int n = (int)regs->ebx;
    struct proc *p = current_proc;
    uint32_t old_brk = p->p_brk;
    uint32_t new_brk = old_brk + n;
    for (; new_brk > p->p_brk_max; p->p_brk_max += PAGE_SIZE) {
        void *page = pmm_alloc_page();
        if (!page) {
            regs->eax = (uint32_t)-1;
            return;
        }
        map_page((page_dir_entry_t *)p->p_page_dir,
                 p->p_brk_max, (uint32_t)page,
                 PAGE_SIZE, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }
    p->p_brk  = new_brk;
    regs->eax = old_brk;
}

static void sys_fork(struct trapframe *regs) {
    struct proc *parent = current_proc;

    struct proc *child = pmm_alloc_page();
    if (!child) {
        regs->eax = (uint32_t)-1;
        return;
    }
    memset(child, 0, sizeof(struct proc));

    static uint32_t next_pid = 2;
    child->p_id = next_pid++;

    uint32_t kstack = (uint32_t)pmm_alloc_page();
    if (!kstack) { pmm_free_page(child); regs->eax = (uint32_t)-1; return; }
    child->p_kernel_stack = kstack + PAGE_SIZE;

    child->p_brk        = parent->p_brk;
    child->p_brk_max    = parent->p_brk_max;
    child->p_stack_base = parent->p_stack_base;
    child->parent       = parent;
    child->p_state      = PROC_READY;

    child->p_ctx        = *regs;
    child->p_ctx.eax    = 0;
    child->p_ctx.cs     = 0x1B;
    child->p_ctx.ss     = 0x23;
    child->p_ctx.ds     = 0x23;
    child->p_ctx.es     = 0x23;
    child->p_ctx.fs     = 0x23;
    child->p_ctx.gs     = 0x23;
    child->p_ctx.eflags = regs->eflags | 0x200;

    child->next_child  = parent->child_head;
    parent->child_head = child;

    proc_build_resume_frame(child);

    uint32_t *parent_pd = (uint32_t *)parent->p_page_dir;
    uint32_t *child_pd  = pmm_alloc_page();
    if (!child_pd) {
        pmm_free_page((void *)kstack);
        pmm_free_page(child);
        regs->eax = (uint32_t)-1;
        return;
    }
    memset(child_pd, 0, PAGE_SIZE);

    uint32_t *kpd   = get_kernel_page_dir();
    int       ncopied = 0;

    for (int i = 0; i < 1024; i++) {
        if (parent_pd[i] == kpd[i]) {
            child_pd[i] = parent_pd[i];
            continue;
        }
        if (!(parent_pd[i] & PAGE_PRESENT))
            continue;

        uint32_t *ppt = (uint32_t *)(parent_pd[i] & ~0xFFFu);
        uint32_t *cpt = pmm_alloc_page();
        if (!cpt) {
            printf("fork: OOM pt[%d], sharing\n", i);
            child_pd[i] = parent_pd[i];
            continue;
        }
        memset(cpt, 0, PAGE_SIZE);

        for (int j = 0; j < 1024; j++) {
            if (!(ppt[j] & PAGE_PRESENT))
                continue;
            if (!(ppt[j] & PAGE_USER)){
                cpt[j] = ppt[j];
                continue;
            }

            uint32_t paddr = ppt[j] & ~0xFFFu;
            uint8_t *dest  = pmm_alloc_page();
            if (!dest) {
                printf("fork: OOM page copy, sharing\n");
                cpt[j] = ppt[j];
                continue;
            }
            memcpy(dest, (uint8_t *)paddr, PAGE_SIZE);
            cpt[j] = ((uint32_t)dest) | (ppt[j] & 0xFFFu);
            ncopied++;
        }
        child_pd[i] = ((uint32_t)cpt) | (parent_pd[i] & 0xFFFu);
    }

    child->p_page_dir = (uint32_t)child_pd;

    for (int i = 0; i < 16; i++)
        if (parent->p_files[i])
            child->p_files[i] = parent->p_files[i];

    if (!parent->p_kernel_stack) {
        uint32_t pk = (uint32_t)pmm_alloc_page();
        if (pk) parent->p_kernel_stack = pk + PAGE_SIZE;
    }

    regs->eax = child->p_id;
    sched_enqueue(child);
}

static const syscall_fn syscall_table[] = {
    [SYS_mkdir]         = sys_mkdir,
    [SYS_open]          = sys_open,
    [SYS_close]         = sys_close,
    [SYS_read]          = sys_read,
    [SYS_write]         = sys_write,
    [SYS_getdents]      = sys_getdents,
    [SYS_lseek]         = sys_lseek,
    [SYS_clock_gettime] = sys_clock_gettime,
    [SYS_execl]         = sys_execl,
    [SYS_exit]          = sys_exit,
    [SYS_wait]          = sys_wait,
    [SYS_sched_yield]   = sys_sched_yield,
    [SYS_fork]          = sys_fork,
    [SYS_sbrk]          = sys_sbrk,
};

void syscall_handler(struct trapframe *regs) {
    uint32_t call = regs->eax;
    if (call < SYSCALL_TABLE_LENGTH(syscall_table) && syscall_table[call])
        syscall_table[call](regs);
    else
        regs->eax = (uint32_t)-1; /* return -1 */
}