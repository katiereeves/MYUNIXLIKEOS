#include "sys/time.h"
#include "sys/syscall.h"
#include "stdio.h"

time_t time(time_t *t) {
    struct timespec ret;
    syscall(SYS_clock_gettime, CLOCK_REALTIME, (long)&ret, 0, 0, 0);
    if(t)
        *t = ret.tv_sec;
    return ret.tv_sec;
}