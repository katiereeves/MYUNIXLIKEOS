/* subject to changes */

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "stdarg.h"
#include "stdint.h"

struct trapframe {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t trapno, err_code;
    uint32_t eip, cs, eflags;
    uint32_t useresp;
    uint32_t ss;
} __attribute__((packed));

/* files and io */
#define SYS_read           0x0000   /* for: read(fd, buf, count) - returns: bytes read */
#define SYS_write          0x0001   /* for: write(fd, buf, count) - returns: bytes written */
#define SYS_open           0x0002   /* for: open(path, flags, mode) - returns: fd */
#define SYS_close          0x0003   /* for: close(fd) - returns: 0 on success */
#define SYS_stat           0x0004   /* for: stat(path, statbuf) - returns: 0 on success */
#define SYS_fstat          0x0005   /* for: fstat(fd, statbuf) - returns: 0 on success */
#define SYS_lstat          0x0006   /* for: lstat(path, statbuf) - returns: 0 on success. */
#define SYS_lseek          0x0007   /* for: lseek(fd, offset, whence) - returns: new offset */
#define SYS_dup            0x0008   /* for: dup(fd) - returns: new fd pointing to same file */
#define SYS_dup2           0x0009   /* for: dup2(oldfd, newfd) - returns: newfd. */
#define SYS_pipe           0x000A   /* for: pipe(fds[2]) - returns: 0 on success. */
#define SYS_mkdir          0x000B   /* for: mkdir(path, mode) - returns: 0 on success */
#define SYS_rmdir          0x000C   /* for: rmdir(path) - returns: 0 on success. */
#define SYS_rename         0x000D   /* for: rename(oldpath, newpath) - returns: 0 on success */
#define SYS_truncate       0x000E   /* for: truncate(path, length) - returns: 0 on success. */
#define SYS_ftruncate      0x000F   /* for: ftruncate(fd, length) - returns: 0 on success. */
#define SYS_chmod          0x0010   /* for: chmod(path, mode) - returns: 0 on success. */
#define SYS_fchmod         0x0011   /* for: fchmod(fd, mode) - returns: 0 on success. */
#define SYS_chown          0x0012   /* for: chown(path, uid, gid) - returns: 0 on success. */
#define SYS_fchown         0x0013   /* for: fchown(fd, uid, gid) - returns: 0 on success. */
#define SYS_unlink         0x0014   /* for: unlink(path) - returns: 0 on success. */
#define SYS_mknod          0x0015   /* for: mknod(path, mode, dev) - returns: 0 on success. */
#define SYS_umask          0x0016   /* for: umask(mask) - returns: old mask. */
#define SYS_access         0x0017   /* for: access(path, mode) - returns: 0 on success. */
#define SYS_utime          0x0018   /* for: utime(path, times) - returns: 0 on success. */
#define SYS_utimes         0x0019   /* for: utimes(path, times[2]) - returns: 0 on success. */
#define SYS_link           0x001A   /* for: link(oldpath, newpath) - returns: 0 on success. */
#define SYS_symlink        0x001B   /* for: symlink(target, linkpath) - returns: 0 on success. */
#define SYS_readlink       0x001C   /* for: readlink(path, buf, bufsz) - returns: bytes read. */
#define SYS_sync           0x001D   /* for: sync() - No return value. */
#define SYS_fsync          0x001E   /* for: fsync(fd) - returns: 0 on success. */
/* directory traversal */
#define SYS_chdir          0x001F   /* for: chdir(path) - returns: 0 on success. */
#define SYS_getcwd         0x0020   /* for: getcwd(buf, size) - returns: 0 on success. */
#define SYS_getdents       0x0021   /* for: getdents(fd, buf, count) - returns: bytes read. */
/* time */
#define SYS_clock_gettime  0x0022   /* for: clock_gettime(clockid, timespec) - returns: 0 on success */
/* procs */
#define SYS_execl          0x0023   /* for: int execl(const char *path, const char *arg0, ...) */
#define SYS_exit           0x0024   /* for: exit(int ret) */
#define SYS_fork           0x0025   /* for: fork(void) */
#define SYS_wait           0x0026   /* for: wait() */
#define SYS_sched_yield    0x0027   /* for yield() */
/* memory */
#define SYS_brk            0x0028   /* for: sbrk(intptr_t incr) */
#define SYS_sbrk           0x0029   /* for: sbrk(intptr_t incr) */

/*
 * Usage is sort of unintuitive:
 * systemcall with one value pass: syscall(SYS_xyz, arg, 0, 0, 0, 0);
 * systemcall with two value pass: syscall(SYS_xyz, arg1, arg2, 0, 0, 0);
 * ...
 * systemcall with five value pass: syscall(SYS_xyz, arg1, arg2, arg3, arg4, arg5);
 */
static inline long syscall(int id, long arg1, long arg2, long arg3, long arg4, long arg5) {
    long ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(id), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
        : "memory"
    );
    return ret;
}

#endif   /* syscall.h */