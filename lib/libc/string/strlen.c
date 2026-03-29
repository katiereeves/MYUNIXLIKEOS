#include "stdio.h"
#include <stddef.h>

size_t strlen(const char *s){
    const char *sptr = s;
    for(; *sptr != '\0'; sptr++);
    return (sptr - s);
}