#ifndef _DIRENT_H_
#define _DIRENT_H_

#include "sys/types.h"
#include "limits.h"

#define DT_DIR 4
#define DT_REG 8

typedef unsigned short reclen_t;

struct dirent{
    ino_t d_ino;    /* File serial number. */
    char  d_name[]; /* Filename string of entry. */
};


struct posix_dent{
    ino_t    d_ino;    /*  File serial number. */
    reclen_t d_reclen; /*  Length of this entry, including trailing
                          padding if necessary. See posix_getdents(). */
    uint8_t d_type;    /* File type or unknown-file-type indication. */
    char    d_name[NAME_MAX+1];  /* Filename string of this entry. */
};

typedef struct {
    int    fd;
    struct posix_dent buf;
} DIR;

DIR               *opendir(const char *path);
struct posix_dent *readdir(DIR *dir);
int                closedir(DIR *dir);
ssize_t posix_getdents(int fildes, void *buf, size_t nbyte, int flags);

#endif /* dirent.h */