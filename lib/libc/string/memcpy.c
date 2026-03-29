#include "string.h"

void *memcpy(void *restrict dest, const void *restrict src, size_t n){
    char *sptr = (char *)src;
    char *dptr = (char *)dest;
    for(; n--; *dptr++ = *sptr++);
    return dest;
}