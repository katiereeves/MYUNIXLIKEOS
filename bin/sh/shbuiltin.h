#ifndef SH_BUILTIN_H
#define SH_BUILTIN_H

#include <stdint.h>
#include "vfs.h"

int cmd_touch(int argc, char** argv);
int cmd_cd(int argc, char** argv);
void cmd_clear();

#endif /* shbuiltin.h */
