#include "commands.h"
#include "stdio.h"
#include "string.h"

extern void terminal_write(const char*);

int cmd_help(int argc, char** argv) {
    if (argc < 2) {
        terminal_write("Available: cat, cd, echo, fs, grep, help, ls, mkdir, touch, clear\n");
        terminal_write("Usage: help [command]\n");
        return 0;
    }

    char* cmd = argv[1];

    if (!strcmp(cmd, "ls")) {
        printf("ls [-a]: List files. -a shows . and ..\n");
    } else if (!strcmp(cmd, "cd")) {
        printf("cd [dir]: Change directory.\n");
    } else if (!strcmp(cmd, "cat")) {
        printf("cat [file]: Read file content.\n");
    } else if (!strcmp(cmd, "fs")) {
        printf("fs [type]: Supports ntfs, exfat, fat32, ext4, apfs.\n");
    } else if (!strcmp(cmd, "echo")) {
        printf("echo [text]: Print text to terminal.\n");
    } else if (!strcmp(cmd, "grep")) {
        printf("grep [pattern] [file]: Search in file.\n");
    } else if (!strcmp(cmd, "mkdir")) {
        printf("mkdir [name]: Create directory.\n");
    } else if (!strcmp(cmd, "touch")) {
        printf("touch [file]: Create empty file.\n");
    } else if (!strcmp(cmd, "help")) {
        printf("help [cmd]: Detailed info on commands.\n");
    } else {
        printf("Unknown command.\n");
    }

    return 0;
}