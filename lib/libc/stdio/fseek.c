#include "stdio.h"
#include "sys/syscall.h"

int fseek(FILE* stream, long offset, int whence) {
    if (!stream) return -1;

    long newoff = syscall(SYS_lseek, (long)stream->fileno,
                          offset, (long)whence, 0, 0);
    if (newoff < 0) return -1;

    stream->eof = 0;
    return 0;
}