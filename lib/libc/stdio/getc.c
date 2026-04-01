#include "stdio.h"
#include "sys/syscall.h"

int getc(FILE *stream) {
    unsigned char c;
    long n = syscall(SYS_read, (long)stream->fileno, (long)&c, 1, 0, 0);
    if (n <= 0) { stream->eof = 1; return EOF; }
    return (int)c;
}