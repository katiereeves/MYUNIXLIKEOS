#include "sched.h"
#include "sys/syscall.h"

int sched_yield() {
    return (int)syscall(SYS_sched_yield, 0, 0, 0, 0, 0);
}