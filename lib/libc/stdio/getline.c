#include "stdio.h"
#include "sys/types.h"

ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream) {
    if (!lineptr || !*lineptr || !n) return -1;

    char  *buf = *lineptr;
    size_t idx = 0;

    for (;;) {
        int c = getc(stream);
        if (c == EOF) { buf[idx] = '\0'; return idx > 0 ? (ssize_t)idx : -1; }
        if (c == '\n') { buf[idx++] = '\n'; buf[idx] = '\0'; return (ssize_t)idx; }
        if (idx + 1 < *n) buf[idx++] = (char)c;
    }

    return -1;
}