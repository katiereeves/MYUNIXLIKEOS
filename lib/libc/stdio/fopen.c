#include "stdio.h"
#include "string.h"
#include "sys/syscall.h"
#include "fcntl.h"

static FILE file_pool[FOPEN_MAX];

FILE* fopen(const char* path, const char* mode) {
    int flags = 0;

    if      (strcmp(mode, "r")  == 0) flags = O_RDONLY;
    else if (strcmp(mode, "w")  == 0) flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (strcmp(mode, "a")  == 0) flags = O_WRONLY | O_CREAT;
    else if (strcmp(mode, "r+") == 0) flags = O_RDWR;
    else if (strcmp(mode, "w+") == 0) flags = O_RDWR  | O_CREAT | O_TRUNC;
    else return NULL;

    int fd = (int)syscall(SYS_open, (long)path, (long)flags, 0, 0, 0);
    if (fd < 0) return NULL;

    FILE* f   = &file_pool[fd];
    f->fileno = fd;
    f->flags  = flags;
    f->buf    = NULL;
    f->bptr   = NULL;
    f->len    = 0;
    f->free   = 0;
    f->iobf   = _IONBF;
    f->eof    = 0;
    return f;
}