#include "sched.h"
#include "../proc/proc.h"
#include "page.h"
#include "pmm.h"
#include "io.h"
#include "stdio.h"
#include "gdt.h"

struct proc *current_proc   = NULL;
struct proc *run_queue_head = NULL;
uint32_t     current_ksp   = 0;

static struct proc *pending_free_list = NULL;

void sched_defer_free(struct proc *p) {
    __asm__ volatile("cli");
    p->next = pending_free_list;
    pending_free_list = p;
    __asm__ volatile("sti");
}

void sched_enqueue(struct proc *p) {
    if (!p) return;
    
    __asm__ volatile("cli");
    
    p->p_state = PROC_READY;
    if (!run_queue_head) {
        run_queue_head = p;
        p->next = p;
    } else {
        struct proc *curr = run_queue_head;
        while (curr->next != run_queue_head) {
            curr = curr->next;
        }
        curr->next = p;
        p->next = run_queue_head;
    }
    
    __asm__ volatile("sti");
}

void sched_dequeue(struct proc *p) {
    if (!p || !run_queue_head) return;

    __asm__ volatile("cli");

    if (p->next == p) {
        if (run_queue_head == p) run_queue_head = NULL;
    } else {
        struct proc *prev = p;
        while (prev->next != p) prev = prev->next;
        prev->next = p->next;
        if (run_queue_head == p) run_queue_head = p->next;
    }
    p->next = NULL;

    __asm__ volatile("sti");
}

void sched_tick(struct trapframe *tf) {
    /* don't fear the reaper */
    struct proc *curr_free = pending_free_list;
    pending_free_list = NULL;
    for (; curr_free;) {
        struct proc *next = curr_free->next;
        if (curr_free->p_kernel_stack)
            pmm_free_page((void *)(curr_free->p_kernel_stack - PAGE_SIZE));
        pmm_free_page(curr_free);
        curr_free = next;
    }

    if (!run_queue_head) return;

    if (current_proc && current_proc->p_state == PROC_RUNNING) {
        current_proc->p_ctx = *tf;
        current_proc->p_ksp = (uint32_t)tf;
        current_proc->p_state = PROC_READY;
    }

    struct proc *next = run_queue_head->next;
    struct proc *stop = run_queue_head;
    do {
        if (next->p_state == PROC_READY)
            goto found;
        next = next->next;
    } while (next != stop);

    if (run_queue_head->p_state != PROC_READY) {
        current_ksp = current_proc ? current_proc->p_ksp : current_ksp;
        return;
    }
    next = run_queue_head;

found:
    run_queue_head        = next;
    current_proc          = next;
    current_proc->p_state = PROC_RUNNING;

    tss_set_kernel_stack(current_proc->p_kernel_stack);
    switch_page_dir(current_proc->p_page_dir);
    current_ksp = current_proc->p_ksp;
}

#define PIT_BASE_FREQ   1193182
#define PIT_CMD         0x43
#define PIT_CHANNEL0    0x40
#define PIC_MASTER_DATA 0x21

void sched_init(uint32_t hz) {
    uint32_t divisor = PIT_BASE_FREQ / hz;
    outb(PIT_CMD,      0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)(divisor >> 8));

    uint8_t mask = inb(PIC_MASTER_DATA);
    outb(PIC_MASTER_DATA, mask & ~0x01);
}