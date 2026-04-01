#include "string.h"

void *memmove(void *dest, const void *src, size_t n) {
    char *dptr = (char *)dest;
    const char *sptr = (const char *)src;

    if (dptr < sptr) {
        for (; n--; *dptr++ = *sptr++);
    } else {
        dptr  += n;
        sptr  += n;
        for (; n--; *--dptr = *--sptr);
    }

    return dest;
}