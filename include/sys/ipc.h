/* Used for XSI interprocess communication.
 * While not needed for a while its
 * simple to write so I threw it in */

#ifndef _SYS_IPC_H_
#define _SYS_IPC_H_

#include "sys/types.h"

typedef int32_t key_t;

struct ipc_perm {
    uid_t   uid;   /* Owner's user ID */
    gid_t   gid;   /* Owner's group ID */
    uid_t   cuid;  /* Creator's user ID  */
    gid_t   cgid;  /* Creator's group ID */
    mode_t  mode;  /* Read/write permission */
};

/* Flags for shmget(), msgget(), or semget() */
#define IPC_CREAT   0x0200  /* Create entry if key does not exist. */
#define IPC_EXCL    0x0400  /* Fail if key exists.                 */
#define IPC_NOWAIT  0x0800  /* Error if request must wait.         */

/* Key */
#define IPC_PRIVATE ((key_t)0)  /* Private key. */

/* Control commands */
#define IPC_RMID    0  /* Remove identifier. */
#define IPC_SET     1  /* Set options.       */
#define IPC_STAT    2  /* Get options.       */

key_t ftok(const char *, int);

#endif /* sys/ipc.h */