#include "unistd.h"
#include "sys/syscall.h"

int close(int fildes) {
    return (int)syscall(SYS_close, (long)fildes, 0, 0, 0, 0);
}