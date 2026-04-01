#include "stdio.h"
#include "sys/syscall.h"

int fclose(FILE* stream) {
    if (!stream) return EOF;
    int ret = (int)syscall(SYS_close, (long)stream->fileno, 0, 0, 0, 0);
    stream->buf  = NULL;
    stream->bptr = NULL;
    stream->len  = 0;
    stream->eof  = 1;
    return ret == 0 ? 0 : EOF;
}