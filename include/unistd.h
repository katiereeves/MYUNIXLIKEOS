#ifndef _UNISTD_H_
#define _UNISTD_H_

#include "sys/types.h"

ssize_t write(int fildes, const void *buf, size_t nbyte);
int execl(const char *path, ...);
ssize_t readline(char *buf, size_t n);

#endif /* unistd.h */