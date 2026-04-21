#ifndef _SCHED_H_
#define _SCHED_H_

#include "../proc/proc.h"

#define SCHED_MAX_PROCS 16
extern struct proc *run_queue_head;
extern uint32_t current_ksp;

void sched_enqueue(struct proc *p);
void sched_dequeue(struct proc *p);
void sched_tick(struct trapframe *tf);
void sched_init(uint32_t hz);
void sched_defer_free(struct proc *p);

#endif /* sched.h */