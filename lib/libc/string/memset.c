#include "string.h"

void *memset(void *s, int c, size_t n){
    for(char *sptr = (char *)s; n--; *sptr = (unsigned char)c);
    return s;
}