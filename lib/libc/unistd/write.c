
#include "sys/types.h"
#include "sys/syscall.h"

ssize_t write(int fildes, const void *buf, size_t nbyte){
    return syscall(SYS_write, (long)fildes, (long)buf, (long)nbyte, 0, 0);
}