#include "stdio.h"
#include "unistd.h"

int putc(int c, FILE *stream){
    char ch = (char)c;
    return write(1, &ch, 1);
}