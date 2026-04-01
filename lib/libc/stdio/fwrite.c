#include "stdio.h"
#include "sys/syscall.h"

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream) return 0;

    long n = syscall(SYS_write, (long)stream->fileno,
                     (long)ptr, (long)(size * nmemb), 0, 0);
    if (n <= 0) return 0;

    return (size_t)n / size;
}