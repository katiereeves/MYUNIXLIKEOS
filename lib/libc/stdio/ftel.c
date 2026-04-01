#include "stdio.h"
#include "sys/syscall.h"

long ftell(FILE* stream) {
    if (!stream) return -1L;
    return (long)syscall(SYS_lseek, (long)stream->fileno,
                         0L, (long)SEEK_CUR, 0, 0);
}