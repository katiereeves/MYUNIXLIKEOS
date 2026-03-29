#include "commands.h"

extern void terminal_write(const char*);

int cmd_echo(int argc, char** argv) {
    if (argc < 2) {
        // Just print a newline if no arguments are provided
        terminal_write("\n");
        return 0;
    }

    // Start from argv[1] because argv[0] is the command name "echo"
    for (int i = 1; i < argc; i++) {
        terminal_write(argv[i]);

        // Add a space between words, but not after the last word
        if (i < argc - 1) {
            terminal_write(" ");
        }
    }

    // End with a newline
    terminal_write("\n");
    return 0;
}