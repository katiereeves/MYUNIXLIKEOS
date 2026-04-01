#include "stdio.h"
#include "sys/syscall.h"

void rewind(FILE* stream) {
    if (!stream) return;
    syscall(SYS_lseek, (long)stream->fileno, 0L, (long)SEEK_SET, 0, 0);
    stream->bptr = stream->buf;
    stream->eof  = 0;
}