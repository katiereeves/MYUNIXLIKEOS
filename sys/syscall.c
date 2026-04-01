/* TODO: possibly move table functions out of here soon... */
#include <stdint.h>
#include <stdio.h>
#include "sys/syscall.h"
#include "fcntl.h"
#include "vfs.h"
#include "nvram.h"
#include "sys/time.h"
#include "elf.h"
#include "io.h"
#include "unistd.h"

/* syscall number: regs->eax
 * arg1: regs->ebx
 * arg2: regs->ecx
 * arg3: regs->edx
 * arg4: regs->esi
 * arg5: regs->edi
 */
struct trapframe {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t trapno, err_code;
    uint32_t eip, cs, eflags;
} __attribute__((packed));

#define SYSCALL_TABLE_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

typedef void (*syscall_fn)(struct trapframe *);

static void sys_mkdir(struct trapframe *regs){
    vfs_mkdir((const char *)regs->ebx);
}

static void sys_open(struct trapframe *regs){
    const char *path = (const char *)regs->ebx;
    int flags = (int)regs->ecx;

    if (flags & O_CREAT)
        if (!vfs_lookup(path))
            vfs_create(path);

    regs->eax = (uint32_t)vfs_open(path, flags);
}

static void sys_close(struct trapframe *regs){
    regs->eax = (uint32_t)vfs_close((int)regs->ebx);
}

static void sys_read(struct trapframe *regs){
    int fd          = (int)regs->ebx;
    void *buf       = (void *)regs->ecx;
    uint32_t count  = regs->edx;

    if (fd == 0) {
        char *b = (char *)buf;
        for (uint32_t i = 0; i < count; i++)
            b[i] = keyboard_getchar();
        regs->eax = count;
        return;
    }

    regs->eax = (uint32_t)vfs_read(fd, buf, count);
}

static void sys_getdents(struct trapframe *regs){
    regs->eax = (uint32_t)vfs_getdents(
        (int)regs->ebx,
        (void *)regs->ecx,
        regs->edx
    );
}

static void sys_lseek(struct trapframe *regs){
    int fd      = (int)regs->ebx;
    long offset = (long)regs->ecx;
    int whence  = (int)regs->edx;

    int size = vfs_fd_size(fd);
    if (size < 0) {
        regs->eax = (uint32_t)-1;
        return;
    }

    int cur = vfs_fd_offset(fd);
    int newoff;

    switch (whence) {
        case SEEK_SET: newoff = (int)offset;        break;
        case SEEK_CUR: newoff = cur + (int)offset;  break;
        case SEEK_END: newoff = size + (int)offset; break;
        default: regs->eax = (uint32_t)-1; return;
    }

    if (newoff < 0 || newoff > size) {
        regs->eax = (uint32_t)-1;
        return;
    }

    vfs_fd_set_offset(fd, (uint32_t)newoff);
    regs->eax = (uint32_t)newoff;
}

static void handle_ansi(const char *buf, uint32_t count, uint32_t *ip){
    uint32_t i = *ip + 2; /* skip esc [ */

    int params[2] = {0, 0};
    int nparam    = 0;
    while (i < count) {
        char d = buf[i];
        if (d >= '0' && d <= '9') {
            params[nparam] = params[nparam] * 10 + (d - '0');
            i++;
        } else if (d == ';') {
            nparam = (nparam + 1) % 2;
            i++;
        } else
            break;
    }

    if (i >= count) {
        *ip = i;
        return;
    }
    char cmd = buf[i];

    switch (cmd) {
        case 'H':
        case 'f':
            move_cursor(
                params[1] > 0 ? params[1] - 1 : 0,
                params[0] > 0 ? params[0] - 1 : 0
            );
            break;
        case 'A':
            move_cursor((int)terminal_col, (int)terminal_row - (params[0] ? params[0] : 1));
            break;
        case 'B':
            move_cursor((int)terminal_col, (int)terminal_row + (params[0] ? params[0] : 1));
            break;
        case 'C':
            move_cursor((int)terminal_col + (params[0] ? params[0] : 1), (int)terminal_row);
            break;
        case 'D':
            move_cursor((int)terminal_col - (params[0] ? params[0] : 1), (int)terminal_row);
            break;
        case 'J':
            if (params[0] == 2) {
                for (size_t col = 0; col < 80; col++)
                    for (size_t row = 0; row < 25; row++) {
                        move_cursor(col, row);
                        terminal_putc(' ');
                    }
                move_cursor(0, 0);
            }
            break;
        case 'K': {
            int saved_col = (int)terminal_col;
            int saved_row = (int)terminal_row;
            terminal_clear_line(saved_row);
            move_cursor(saved_col, saved_row);
            break;
        }
        default:
            break;
    }

    *ip = i;
}

static void sys_write(struct trapframe *regs){
    int fd          = (int)regs->ebx;
    const char *buf = (const char *)regs->ecx;
    uint32_t count  = regs->edx;

    if ((fd == 1 || fd == 2) && buf && count) {
        for (uint32_t i = 0; i < count; i++) {
            char c = buf[i];
            if (c == '\033' && i + 1 < count && buf[i + 1] == '[') {
                handle_ansi(buf, count, &i);
            } else {
                switch (c) {
                    case '\b':
                    case 0x7F: terminal_backspace(); break;
                    default:   terminal_putc(c);     break;
                }
            }
        }
        regs->eax = count;
    } else if (fd > 2 && buf && count) {
        regs->eax = (uint32_t)vfs_write(fd, buf, count);
    } else {
        regs->eax = (uint32_t)-1;
    }
}

#define IS_LEAP(y) (((y) % 4 == 0 && (y) % 100 != 0) || ((y) % 400 == 0))

static void sys_clock_gettime(struct trapframe *regs){
    if (regs->ebx != CLOCK_REALTIME) {
        regs->eax = (uint32_t)-1;
        return;
    }

    char nvbuf[128];
    struct nvram_t *nv = (struct nvram_t *)nvbuf;
    readNVRAM(nvbuf);

    struct timespec *tp = (struct timespec *)regs->ecx;

    int sec   = bcd(nv->rtc_sec);
    int min   = bcd(nv->rtc_min);
    int hour  = bcd(nv->rtc_hour);
    int day   = bcd(nv->rtc_day);
    int month = bcd(nv->rtc_month);
    int year  = bcd(nv->century_BCD) * 100 + bcd(nv->rtc_year);

    uint8_t days_per_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    time_t total_days = 0;
    for (int y = 1970; y < year; y++)
        total_days += IS_LEAP(y) ? 366 : 365;

    for (int m = 0; m < month - 1; m++) {
        total_days += days_per_month[m];
        if (m == 1 && IS_LEAP(year))
            total_days += 1;
    }

    total_days += day - 1;
    tp->tv_sec  = total_days * 86400 + hour * 3600 + min * 60 + sec;
    tp->tv_nsec = 0;
    regs->eax   = 0;
}

static void sys_execl(struct trapframe *regs){
    regs->eax = (uint32_t)elf_exec(
        (const char *)regs->ebx,
        (const char **)regs->ecx
    );
}

static void sys_exit(struct trapframe *regs){
    /* just start shell for now */
    elf_exec("sh", NULL);
}

static const syscall_fn syscall_table[] = {
    [SYS_mkdir]         = sys_mkdir,
    [SYS_open]          = sys_open,
    [SYS_close]         = sys_close,
    [SYS_read]          = sys_read,
    [SYS_write]         = sys_write,
    [SYS_getdents]      = sys_getdents,
    [SYS_lseek]         = sys_lseek,
    [SYS_clock_gettime] = sys_clock_gettime,
    [SYS_execl]         = sys_execl,
    [SYS_exit]          = sys_exit,
};

void syscall_handler(struct trapframe *regs){
    uint32_t call = regs->eax;
    if (call < SYSCALL_TABLE_LENGTH(syscall_table) && syscall_table[call])
        syscall_table[call](regs);
    else
        regs->eax = (uint32_t)-1; /* return -1 */
}