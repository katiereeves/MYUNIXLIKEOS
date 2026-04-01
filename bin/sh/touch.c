#include "shbuiltin.h"
#include "stdio.h"
#include "fcntl.h"

int cmd_touch(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: touch <filename>\n");
        return -1;
    }
    FILE *fp = fopen(argv[1], "w");
    if (!fp) {
        printf("touch: cannot create file: %s\n", argv[1]);
        return -1;
    }
    fclose(fp);
}