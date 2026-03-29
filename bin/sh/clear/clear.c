#include "stdio.h"
#include <stdint.h>

extern int terminal_row;
extern int terminal_col;

/* pretty hacky for now,
 * should just throw a bunch
 * of new lines when we support
 * scrolling.
 */
void cmd_clear(){
    terminal_row = 0;
    terminal_col = 0;
    for(int32_t i = 0; i < 80; i++)
        for(int32_t j = 0; j < 25; j++)
            putchar(' ');

    terminal_row = 0;
    terminal_col = 0;
}
