#include "sys/stat.h"
#include "sys/types.h"
#include "sys/syscall.h"

int mkdir(const char *path, mode_t mode){
    return syscall(SYS_mkdir, (long)path, (long)mode, 0, 0, 0);
}