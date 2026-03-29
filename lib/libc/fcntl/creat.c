#include "fcntl.h"
#include "sys/syscall.h"

int creat(const char *path, mode_t mode){
    return syscall(SYS_open, (long)path, (long)mode, (long)O_CREAT, 0, 0);
}