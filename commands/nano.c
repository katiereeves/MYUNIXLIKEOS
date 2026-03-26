#include "commands.h"
#include "vfs.h"
#include "string.h"
#include "pmm.h"
#include "io.h"

// DIRECT LINK TO KERNEL.C FUNCTIONS
extern void terminal_write(const char* data);
extern void read_line(char* buffer, int size);

// Now your cmd_nano code follows...
// Helper to show the help menu (:G)
void nano_help() {
    terminal_write("\n--- Nano Help ---\n");
    terminal_write(":X Exit      :O WriteOut   :G Get Help   :W Where Is\n");
    terminal_write(":R Read File :C Cur Pos    :K Cut Text   :U UnCut\n");
    terminal_write(":Y Prev Pg   :V Next Pg    :J Justify    :T To Spell\n");
    terminal_write("-----------------\n");
}

int cmd_nano(int argc, char** argv) {
    if (argc < 2) {
        terminal_write("Usage: nano <filename>\n");
        return -1;
    }

    const char* path = argv[1];
    vnode_t* file = vfs_lookup(path);

    if (!file) {
        if (k_touch(path) != 0) {
            terminal_write("nano: error creating file\n");
            return -1;
        }
        file = vfs_lookup(path);
    }

    char input[256];
    char buffer[4096];
    size_t len = 0;

    // Initial Load
    if (file->content && file->size > 0) {
        while (len < file->size && len < sizeof(buffer) - 1) {
            buffer[len] = file->content[len];
            len++;
        }
    }
    buffer[len] = '\0';

    terminal_write("nano: editing ");
    terminal_write(path);
    terminal_write(" (Type :G for help)\n");

    while (1) {
        terminal_write("> ");
        read_line(input, sizeof(input));

        // Command Mode (starts with :)
        if (input[0] == ':') {
            char cmd = input[1];

            if (cmd == 'X') { // Exit
                terminal_write("Save changes? (y/n): ");
                read_line(input, sizeof(input));
                if (input[0] == 'y') goto save_logic;
                return 0;
            } 
            else if (cmd == 'O') { // WriteOut (Save)
            save_logic:
                if (!file->content) file->content = (char*)pmm_alloc_z(4096);
                for (size_t i = 0; i < len; i++) file->content[i] = buffer[i];
                file->content[len] = '\0';
                file->size = len;
                terminal_write("nano: written to disk\n");
                if (cmd == 'X') return 0;
            }
            else if (cmd == 'G') { // Help
                nano_help();
            }
            else if (cmd == 'W') { // Where Is (Search)
                terminal_write("Search for: ");
                read_line(input, sizeof(input));
                // Minimalist search: just tells you if it exists
                if (strstr(buffer, input)) terminal_write("Match found!\n");
                else terminal_write("No match.\n");
            }
            else if (cmd == 'C') { // Cur Pos (Status)
                // In a VFS/i686 environment, we just report size
                terminal_write("File size: ");
                // (You'd use your itoa/printf here once Katie finishes hers)
                terminal_write(" chars\n");
            }
            else if (cmd == 'R') { // Read File
                terminal_write("nano: :R is a future feature\n");
            }
            else {
                terminal_write("nano: Command not implemented yet.\n");
            }
            continue;
        }

        // Standard text entry
        size_t line_len = strlen(input);
        if (len + line_len + 1 < sizeof(buffer)) {
            for (size_t i = 0; i < line_len; i++) buffer[len++] = input[i];
            buffer[len++] = '\n';
            buffer[len] = '\0';
        } else {
            terminal_write("nano: buffer full\n");
        }
    }
}