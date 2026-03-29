#include "string.h"
#include "stddef.h"

char *strstr(const char *s1, const char *s2){
    if (!*s2)
        return (char *)s1;

    for (; *s1; s1++) {
        if (*s1 != *s2) continue;

        const char *n = s2;
        for (const char *h = s1; *h && *n && *h == *n; h++) n++;
        if (!*n)
            return (char *)s1;
    }
    return NULL;
}