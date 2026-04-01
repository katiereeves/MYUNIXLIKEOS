/* Subject to change */

#include "stdio.h"
#include "stddef.h"

extern void terminal_putc(char);

int putchar(int c){
    FILE *fp = NULL;
    return putc(c, fp); // set to null for now
}
