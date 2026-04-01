#include "unistd.h"
#include "sys/syscall.h"
#include "stdio.h"

off_t lseek(int fildes, off_t offset, int whence) {
    return (off_t)syscall(SYS_lseek, (long)fildes, (long)offset, (long)whence, 0, 0);
}