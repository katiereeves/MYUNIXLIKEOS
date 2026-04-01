/* WIP:
 * Not ideal, but it "works" for now.
 * Non-standard but working towards it.
 * Much of this needs to be abstracted.
 *
 * TODO:
 * Short-term goals:
 *  - Make lines not wrap when too long.
 * Long-term goals:
 *  - Motion commands.
 */

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "fcntl.h"


#define LINE_MAX 2048
#define VI_BUF   4096
#define VI_ROWS  24

static const char *inst_str = "-- INSERT --";
static const char *cmd_str  = "-- COMMAND --";

/* cursor in screen space */
static int cur_row = 0;
static int cur_col = 0;

/* cursor in buffer space */
static size_t buf_cur = 0;

static void vi_move(int row, int col) {
    printf("\033[%d;%dH", row + 1, col + 1);
}

static void vi_clear(void) {
    printf("\033[2J");
    vi_move(0, 0);
}

static void vi_clear_line(void) {
    printf("\033[2K");
}

static void vi_print_status(const char *msg) {
    int row = cur_row;
    int col = cur_col;
    vi_move(VI_ROWS, 0);
    vi_clear_line();
    printf("%s", msg);
    vi_move(cur_row, cur_col);
}

/* convert buffer offset to screen row/col */
static void vi_offset_to_screen(const char *buf, size_t offset,
                                  int *row, int *col) {
    int r = 0, c = 0;
    for (size_t i = 0; i < offset; i++) {
        if (buf[i] == '\n') { r++; c = 0; }
        else                { c++;        }
    }
    *row = r;
    *col = c;
}

/* convert screen row/col to buffer offset */
static size_t vi_screen_to_offset(const char *buf, size_t len,
                                   int row, int col) {
    int r = 0, c = 0;
    for (size_t i = 0; i < len; i++) {
        if (r == row && c == col) return i;
        if (buf[i] == '\n') { r++; c = 0; }
        else                { c++;        }
    }
    return len;
}

/* length of line at given row */
static int vi_line_len(const char *buf, size_t len, int row) {
    int r = 0, c = 0, max = 0;
    for (size_t i = 0; i < len; i++) {
        if (r == row) { max = c; }
        if (buf[i] == '\n') { if (r == row) return max; r++; c = 0; }
        else                { c++; }
    }
    if (r == row) return c;
    return 0;
}

/* total number of rows in buffer */
static int vi_row_count(const char *buf, size_t len) {
    int r = 0;
    for (size_t i = 0; i < len; i++)
        if (buf[i] == '\n') r++;
    return r;
}

static void vi_draw(const char *name, const char *buf, size_t len) {
    vi_clear();

    size_t buf_pos      = 0;
    size_t content_rows = 0;

    for (int row = 0; row < VI_ROWS-1; row++) {
        vi_move(row, 0);
        if (buf_pos < len) {
            while (buf_pos < len && buf[buf_pos] != '\n')
                putchar(buf[buf_pos++]);
            if (buf_pos < len)
                buf_pos++;
            content_rows++;
        } else {
            putchar('~');
        }
    }

    vi_move(VI_ROWS, 0);
    printf("%s", name);

    /* place cursor at buf_cur position */
    vi_offset_to_screen(buf, buf_cur, &cur_row, &cur_col);
    vi_move(cur_row, cur_col);
}

/* insert character at buf_cur */
static void vi_insert_char(char *buf, size_t *len, char c) {
    if (*len + 1 >= VI_BUF) return;
    memmove(buf + buf_cur + 1, buf + buf_cur, *len - buf_cur + 1);
    buf[buf_cur] = c;
    (*len)++;
    buf_cur++;
}

/* delete character before buf_cur */
static void vi_delete_char(char *buf, size_t *len) {
    if (buf_cur == 0 || *len == 0) return;
    buf_cur--;
    memmove(buf + buf_cur, buf + buf_cur + 1, *len - buf_cur);
    (*len)--;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: vi <filename>\n");
        return -1;
    }

    const char *filename = argv[1];

    char buf[VI_BUF];
    size_t len = 0;

    FILE *f = fopen(filename, "r");
    if (f) {
        len = fread(buf, 1, VI_BUF - 1, f);
        buf[len] = '\0';
        fclose(f);
    } else {
        f = fopen(filename, "w");
        if (!f) {
            printf("vi: cannot create file: %s\n", filename);
            return -1;
        }
        fclose(f);
        buf[0] = '\0';
        len    = 0;
    }

    /* start cursor at end of file */
    buf_cur = len;

    bool   cmd_mode  = false;
    char   input[LINE_MAX];
    size_t input_len = 0;

    vi_draw(filename, buf, len);
    vi_print_status(inst_str);
    vi_offset_to_screen(buf, buf_cur, &cur_row, &cur_col);
    vi_move(cur_row, cur_col);

    for (;;) {
        char c = (char)getchar();

        if (c == 0x1B) { /* toggle command mode */
            cmd_mode  = !cmd_mode;
            input_len = 0;
            input[0]  = '\0';
            if (cmd_mode) {
                vi_print_status(cmd_str);
                vi_move(VI_ROWS, (int)strlen(cmd_str) + 1);
            } else {
                vi_draw(filename, buf, len);
                vi_print_status(inst_str);
                vi_move(cur_row, cur_col);
            }
            continue;
        }

        if (c == '\n') {
            input[input_len] = '\0';

            if (cmd_mode) {
                if (strcmp(input, ":wq") == 0) {
                    f = fopen(filename, "w");
                    if (!f) {
                        vi_print_status("vi: cannot open file for writing");
                        continue;
                    }
                    fwrite(buf, 1, len, f);
                    fclose(f);
                    vi_clear();
                    return 0;
                }
                if (strcmp(input, ":q!") == 0) {
                    vi_clear();
                    return 0;
                }
                char err[LINE_MAX];
                size_t ei = 0;
                for (const char *p = "Not an editor command: "; *p;)
                    err[ei++] = *p++;
                for (size_t j = 0; input[j] && ei < sizeof(err) - 1;)
                    err[ei++] = input[j++];
                err[ei] = '\0';
                vi_print_status(err);

            } else {
                vi_insert_char(buf, &len, '\n');
                vi_draw(filename, buf, len);
                vi_print_status(inst_str);
                vi_move(cur_row, cur_col);
            }

            input_len = 0;
            input[0]  = '\0';
            continue;
        }

        if (c == '\b' || c == 0x7F) {
            if (!cmd_mode && buf_cur > 0) {
                vi_delete_char(buf, &len);
                vi_draw(filename, buf, len);
                vi_print_status(inst_str);
                vi_move(cur_row, cur_col);
            } else if (cmd_mode && input_len > 0) {
                input[--input_len] = '\0';
                /* move cursor back on status line */
                int status_col = (int)strlen(cmd_str) + 1 + (int)input_len;
                vi_move(VI_ROWS, status_col);
                putchar(' ');
                vi_move(VI_ROWS, status_col);
            }
            continue;
        }

        if (cmd_mode) {
            if (input_len < LINE_MAX - 1) {
                input[input_len++] = c;
                input[input_len]   = '\0';
                putchar(c);
            }
        } else {
            vi_insert_char(buf, &len, c);
            vi_draw(filename, buf, len);
            vi_move(cur_row, cur_col);
            vi_print_status(inst_str);
        }
    }
}