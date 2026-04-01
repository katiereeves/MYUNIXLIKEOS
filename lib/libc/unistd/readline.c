#include "unistd.h"
#include "stdio.h"

ssize_t readline(char *buf, size_t n) {
    size_t idx = 0;

    for (;;) {
        char c = getchar();
        if (!c) continue;

        if (c == '\r' || c == '\n') {
            putchar('\n');
            buf[idx] = '\0';
            return (ssize_t)idx;
        }

        if (c == '\b' || c == 0x7F) {
            if (idx > 0) {
                idx--;
                buf[idx] = '\0';
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
            continue;
        }

        if (idx + 1 < n) {
            buf[idx++] = c;
            putchar(c);
        }
    }

    return -1;
}