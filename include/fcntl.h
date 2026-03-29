#ifndef _FCNTL_H_
#define _FCNTL_H_

#include "sys/types.h"  /* mode_t, off_t, pid_t */

/* Commands for fcntl() */
#define F_DUPFD     0   /* Duplicate file descriptor. */
#define F_GETFD     1   /* Get file descriptor flags. */
#define F_SETFD     2   /* Set file descriptor flags. */
#define F_GETFL     3   /* Get file status flags and file access modes. */
#define F_SETFL     4   /* Set file status flags. */
#define F_GETLK     5   /* Get record locking information. */
#define F_SETLK     6   /* Set record locking information. */
#define F_SETLKW    7   /* Set record locking information; wait if blocked. */
#define F_GETOWN    8   /* Get process or process group ID to receive SIGURG signals. */
#define F_SETOWN    9   /* Set process or process group ID to receive SIGURG signals. */

/* File descriptor flags for fcntl() */
#define FD_CLOEXEC  1   /* Close the file descriptor upon execution of an exec family function. */

/* Values for l_type in struct flock (record locking) */
#define F_RDLCK     0   /* Shared or read lock. */
#define F_UNLCK     1   /* Unlock. */
#define F_WRLCK     2   /* Exclusive or write lock. */

/* File creation flags for open() — shall be bitwise-distinct.*/
#define O_CREAT     0x0040  /* Create file if it does not exist. */
#define O_EXCL      0x0080  /* Exclusive use flag. */
#define O_NOCTTY    0x0100  /* Do not assign controlling terminal. */
#define O_TRUNC     0x0200  /* Truncate flag. */

/* File status flags for open() and fcntl() */
#define O_APPEND    0x0400  /* Set append mode. */
#define O_DSYNC     0x1000  /* Write according to synchronized I/O data integrity completion. */
#define O_NONBLOCK  0x0800  /* Non-blocking mode. */
#define O_RSYNC     0x2000  /* Synchronized read I/O operations. */
#define O_SYNC      0x4000  /* Write according to synchronized I/O file integrity completion. */

#define O_ACCMODE   0x0003  /* Mask for file access modes. */

/* File access modes for open() and fcntl() */
#define O_RDONLY    0x0000  /* Open for reading only. */
#define O_WRONLY    0x0001  /* Open for writing only. */
#define O_RDWR      0x0002  /* Open for reading and writing. */

/* Advisory values for posix_fadvise() */
#define POSIX_FADV_NORMAL       0   /* No advice; default behavior. */
#define POSIX_FADV_SEQUENTIAL   1   /* Expect sequential access from lower to higher offsets. */
#define POSIX_FADV_RANDOM       2   /* Expect random order access. */
#define POSIX_FADV_WILLNEED     3   /* Expect access to specified data in the near future. */
#define POSIX_FADV_DONTNEED     4   /* Do not expect access to specified data in the near future. */
#define POSIX_FADV_NOREUSE      5   /* Expect to access specified data once and not reuse it. */

/* Record locking structure for fcntl() */
struct flock {
    short l_type;    /* Type of lock: F_RDLCK, F_WRLCK, F_UNLCK. */
    short l_whence;  /* Flag for starting offset (SEEK_SET, SEEK_CUR, SEEK_END). */
    off_t l_start;   /* Relative offset in bytes. */
    off_t l_len;     /* Size; if 0 then until EOF. */
    pid_t l_pid;     /* Process ID of the process holding the lock; returned with F_GETLK. */
};

int creat(const char *path, mode_t mode);
int fcntl(int fildes, int cmd, ...);
int open(const char *path, int oflag, ...);
int posix_fadvise(int fd, off_t offset, off_t len, int advice);
int posix_fallocate(int fd, off_t offset, off_t len);

#endif /* fcntl.h */