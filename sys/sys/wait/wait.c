#include "sys/wait.h"
#include "sys/syscall.h"
#include "sys/types.h"

pid_t wait(int *stat_loc){
    int pid = syscall(SYS_wait, 0, 0, 0, 0, 0);
    return pid;
}