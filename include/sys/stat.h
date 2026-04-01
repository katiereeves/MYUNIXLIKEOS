#ifndef _STAT_H_
#define _STAT_H_

#include "sys/types.h"

#define S_IRWXU 1
#define S_IRWXG 2
#define S_IRWXO 3

typedef uint32_t mode_t;

int mkdir(const char *path, mode_t mode);

#endif /* stat.h */