#include "commands.h"

extern void terminal_write(const char*);

// A tiny, 4-line replacement for strcmp
int str_equal(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(unsigned char*)s1 == *(unsigned char*)s2;
}

int cmd_help(int argc, char** argv) {
    if (argc < 2) {
        terminal_write("Available: cat, cd, echo, fs, grep, help, ls, mkdir, touch, clear\n");
        terminal_write("Usage: help [command]\n");
        return 0;
    }

    char* cmd = argv[1];

    if (str_equal(cmd, "ls")) {
        terminal_write("ls [-a]: List files. -a shows . and ..\n");
    } else if (str_equal(cmd, "cd")) {
        terminal_write("cd [dir]: Change directory.\n");
    } else if (str_equal(cmd, "cat")) {
        terminal_write("cat [file]: Read file content.\n");
    } else if (str_equal(cmd, "fs")) {
        terminal_write("fs [type]: Supports ntfs, exfat, fat32, ext4, apfs.\n");
    } else if (str_equal(cmd, "echo")) {
        terminal_write("echo [text]: Print text to terminal.\n");
    } else if (str_equal(cmd, "grep")) {
        terminal_write("grep [pattern] [file]: Search in file.\n");
    } else if (str_equal(cmd, "mkdir")) {
        terminal_write("mkdir [name]: Create directory.\n");
    } else if (str_equal(cmd, "touch")) {
        terminal_write("touch [file]: Create empty file.\n");
    } else if (str_equal(cmd, "help")) {
        terminal_write("help [cmd]: Detailed info on commands.\n");
    } else {
        terminal_write("Unknown command.\n");
    }

    return 0;
}