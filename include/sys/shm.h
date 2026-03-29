/* While not needed for a while its
 * simple to write so I threw it in
 */

#ifndef _SMH_H_
#define _SMH_H_

#include "sys/types.h"
#include "sys/ipc.h"
#include "pmm.h" /* PAGE_SIZE */

#define SHM_RDONLY 010000    /* Attach read-only (else read-write). */
#define SHMLBA     PAGE_SIZE /* Segment low boundary address multiple. */
#define SHM_RND    020000    /* Round attach address to SHMLBA. */

typedef unsigned short shmatt_t;

struct shmid_ds{
    struct ipc_perm shm_perm;   /* operation permission structure */
    size_t          shm_segsz;  /* size of segment in bytes */
    pid_t           shm_lpid;   /* process ID of last shared memory operation */
    pid_t           shm_cpid;   /* process ID of creator */
    shmatt_t        shm_nattch; /* number of current attaches */
    time_t          shm_atime;  /* time of last shmat() */
    time_t          shm_dtime;  /* time of last shmdt() */
    time_t          shm_ctime;  /* time of last change by shmctl() */
};

void *shmat(int, const void *, int);
int   shmctl(int, int, struct shmid_ds *);
int   shmdt(const void *);
int   shmget(key_t, size_t, int);

#endif /* sys/smh.h */