#include "string.h"

char *strcpy(char *restrict s1, const char *restrict s2){
    char *s1ptr = (char *)s1;
    char *s2ptr = (char *)s2;
    for(; *s2ptr; *s1ptr++ = *s2ptr++);
    return s1;
}