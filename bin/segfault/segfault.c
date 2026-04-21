/* just to test seg faulting */
#include "stdio.h"

int main(){
    char *j = "test";
    for (char *i = j;; i += 0x1000)
        printf("%i: %c\n", (int)i, *i);
}