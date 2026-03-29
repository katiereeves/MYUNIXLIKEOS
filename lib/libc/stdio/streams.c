/* userspace streams */
#include "stdio.h"

static unsigned char _stdout_buf[2048];
static unsigned char _stderr_buf[2048];
static unsigned char _stdin_buf[2048];

static FILE _stdout_f = {
    .flags = 1,
    .fileno = 1,
    .buf  = _stdout_buf,
    .bptr = _stdout_buf,
    .len  = sizeof(_stdout_buf),
    .free = 0,
    .iobf = _IOLBF,
    .eof  = 0,
};

static FILE _stderr_f = {
    .flags = 1,
    .fileno = 2,
    .buf  = _stderr_buf,
    .bptr = _stderr_buf,
    .len  = sizeof(_stderr_buf),
    .free = 0,
    .iobf = _IONBF,
    .eof  = 0,
};

static FILE _stdin_f = {
    .flags = 0,
    .fileno = 0,
    .buf  = _stdin_buf,
    .bptr = _stdin_buf,
    .len  = sizeof(_stdin_buf),
    .free = 0,
    .iobf = _IOLBF,
    .eof  = 0,
};

FILE *stdout = &_stdout_f;
FILE *stderr = &_stderr_f;
FILE *stdin  = &_stdin_f;