#ifndef _SYS_PROC_H_
#define _SYS_PROC_H_

#include "sys/types.h"
#include "stdint.h"
#include "sys/syscall.h"

typedef enum {
    PROC_RUNNING,
    PROC_READY,
    PROC_SLEEPING,
    PROC_ZOMBIE,
    PROC_DEAD,
    PROC_WAITING
} proc_state_t;

struct proc {
    size_t            p_id;
    proc_state_t      p_state;
    struct trapframe  p_ctx;
    struct proc      *parent;
    struct proc      *next;
    struct proc      *next_child;
    struct proc      *child_head;
    struct file      *p_files[16];
    int               p_exitcode;

    /* Memory layout */
    uint32_t          p_stack_base;
    uint32_t          p_brk;
    uint32_t          p_brk_max;
    uint32_t          p_page_dir;
    uint32_t          p_kernel_stack;
    uint32_t          p_ksp;

    uint32_t          p_sleep_ticks;
};

extern struct proc *current_proc;

void schedule(void);
void proc_run_once(const char *path);
struct proc *proc_from_elf(const char *path);
void free_proc(struct proc *p);
uint32_t *clone_page_directory(uint32_t *parent_pd);

#endif /* sys/proc.h */