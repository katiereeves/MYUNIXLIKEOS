#include "stdio.h"
#include "sys/syscall.h"

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream || stream->eof) return 0;

    long n = syscall(SYS_read, (long)stream->fileno,
                     (long)ptr, (long)(size * nmemb), 0, 0);
    if (n <= 0) { stream->eof = 1; return 0; }

    return (size_t)n / size;
}