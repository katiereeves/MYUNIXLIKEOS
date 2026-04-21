#include "unistd.h"
#include "sys/syscall.h"

pid_t fork(void){
    return syscall(SYS_fork, 0, 0, 0, 0, 0);
}