#include "commands.h"
#include "fcntl.h"

int cmd_touch(int argc, char** argv) {
    return creat(argv[1], 0);
}