#include "unistd.h"
#include "sys/syscall.h"
#include <stdarg.h>

/*
 * execl(path, arg0, arg1, ..., NULL)
 *
 * Collects the varargs into a local argv[] array, then passes:
 *   ebx = path
 *   ecx = argv  (pointer to NULL-terminated array of char*)
 * to SYS_execl.  The kernel copies the strings onto the user stack
 * before jumping to ring 3.
 */
int execl(const char *path, ...) {
    va_list ap;
    const char *args[16];
    int argc = 0;

    va_start(ap, path);
    const char *a;
    for (; (a = va_arg(ap, const char *)) != NULL && argc < 15; args[argc++] = a);
    va_end(ap);
    args[argc] = NULL;

    int ret = syscall(SYS_execl, (long)path, (long)args, 0, 0, 0);
    return ret;
}