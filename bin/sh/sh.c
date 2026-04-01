#include "stdio.h"
#include "shbuiltin.h"
#include "string.h"
#include "limits.h"
#include "stddef.h"
#include "vfs.h"
#include "unistd.h"

static void split_args(char *input, char **argv, int *argc) {
    *argc = 0;
    char in_token = 0;

    while (*input) {
        if (*input == ' ' || *input == '\t') {
            *input = '\0';
            in_token = 0;
        } else if (!in_token) {
            in_token = 1;
            argv[(*argc)++] = input;
            if (*argc >= 16)
                break;
        }
        input++;
    }
}

int main(void) {
    char buf[ARG_MAX];
    char *argv[16];
    int  argc;

    for (;;) {
        memset(buf,  0, sizeof(buf));
        memset(argv, 0, sizeof(argv));
        argc = 0;

        printf("root@dev_null# ");

        if (readline(buf, sizeof(buf)) < 0) continue;
        if (buf[0] == '\0') continue;

        split_args(buf, argv, &argc);
        if (argc == 0) continue;
        /* built in cmd */
        if (strcmp(argv[0], "touch") == 0) { cmd_touch(argc, argv); continue; }
        if (strcmp(argv[0], "cd")    == 0) { cmd_cd(argc, argv);    continue; }
        if (strcmp(argv[0], "clear") == 0) { cmd_clear();           continue; }

        if (strcmp(argv[0], "exec") == 0) {
            if (argc < 2) { printf("exec: missing program name\n"); continue; }
            argv[argc] = NULL;
            execl(argv[1], argv[1], argv[2], argv[3], argv[4],
                  argv[5], argv[6], argv[7], argv[8], argv[9],
                  argv[10], argv[11], argv[12], argv[13], argv[14], NULL);
            continue;
        }

        printf("sh: command not found: %s\n", argv[0]);
    }
}