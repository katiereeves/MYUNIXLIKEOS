#include "stdio.h"
#include "sys/syscall.h"

int getchar(void) {
    unsigned char c;
    long n = syscall(SYS_read, 0L, (long)&c, 1L, 0, 0);
    if (n <= 0) return EOF;
    return (int)c;
}