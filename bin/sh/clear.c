#include "stdio.h"
#include "shbuiltin.h"

/* pretty hacky for now,
 * should just throw a bunch
 * of new lines when we support
 * scrolling.
 */
void cmd_clear(){
    printf("\033[2J\033[H");
}
