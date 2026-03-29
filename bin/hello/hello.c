/* prog to test ELF support */

#include "stdio.h"

int main(int argc, char **argv){
    char **aptr = argv;
    for(; argc--; printf("%s ", *aptr++));
    printf("\nHello, World!\n");
    return 0;
}