#include "dirent.h"
#include "sys/types.h"
#include "sys/syscall.h"

ssize_t posix_getdents(int fildes, void *buf, size_t nbyte, int flags){
    return syscall(SYS_getdents, (long)fildes, (long)buf, (long)nbyte, 0, 0);
}