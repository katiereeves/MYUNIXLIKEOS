#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#include <stdint.h>
#include "stddef.h"

/* integer types */
typedef int32_t  blkcnt_t;    /* used for file block counts */
typedef int32_t  blksize_t;   /* used for block sizes */
typedef uint64_t dev_t;       /* used for device IDs */
typedef uint32_t gid_t;       /* used for group IDs */
typedef uint64_t ino_t;       /* used for file serial numbers */
typedef uint32_t mode_t;      /* used for file attribute bits */
typedef uint32_t nlink_t;     /* used for hard link counts */
typedef int64_t  off_t;       /* used for file sizes and offsets */
typedef int32_t  pid_t;       /* used for process IDs and process group IDs */
typedef long     ssize_t;     /* used for byte counts or error indication */
typedef int64_t  time_t;      /* used for time in seconds since epoch, 2038 compatable */
typedef uint32_t uid_t;       /* used for user IDs */

/* IPC and interprocess */
typedef int32_t  key_t;       /* used for XSI interprocess communication */
typedef int32_t  id_t;        /* used as a general identifier (pid, uid, gid) */

/* timer and clock */
typedef int32_t  clockid_t;   /* used for clock IDs */
typedef int32_t  timer_t;     /* used for timer IDs */
typedef int64_t  suseconds_t; /* used for microseconds in struct timeval */
typedef int64_t  useconds_t;  /* used for microseconds passed to usleep */
typedef int64_t  clock_t;     /* used for processor time (CLOCKS_PER_SEC units) */

/* filesystem */
typedef uint32_t fsblkcnt_t;  /* used for file system block counts */
typedef uint32_t fsfilcnt_t;  /* used for file system file counts */

/* locale and wide characters */
typedef int32_t  wint_t;      /* used for wide character or WEOF */

/* pthread and synchronization */
typedef uint32_t pthread_t;          /* thread ID */
typedef uint32_t pthread_key_t;      /* thread-specific data key */
typedef int32_t  pthread_once_t;     /* one-time initialization control */
typedef struct { uint32_t val; }        pthread_mutex_t;     /* mutex */
typedef struct { uint32_t val; }        pthread_mutexattr_t; /* mutex attributes */
typedef struct { uint32_t val[2]; }     pthread_cond_t;      /* condition variable */
typedef struct { uint32_t val; }        pthread_condattr_t;  /* condition variable attributes */
typedef struct { uint32_t val[4]; }     pthread_rwlock_t;    /* read/write lock */
typedef struct { uint32_t val; }        pthread_rwlockattr_t;/* read/write lock attributes */
typedef struct { uint32_t val; }        pthread_attr_t;      /* thread attributes */
typedef struct { uint32_t val; }        pthread_barrier_t;   /* barrier */
typedef struct { uint32_t val; }        pthread_barrierattr_t; /* barrier attributes */
typedef struct { uint32_t val; }        pthread_spinlock_t;  /* spinlock */

#endif /* types.h */