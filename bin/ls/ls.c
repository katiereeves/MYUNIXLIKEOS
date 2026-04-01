#include "stdio.h"
#include "dirent.h"
#include "fcntl.h"
#include "stdbool.h"
#include "sys/syscall.h"

#define ENTBUFSIZ 1024

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : ".";
    bool a = (argv[1][0] == '-' && argv[1][1] == 'a' ); // proper parser down the line
    if(a && argc == 2)
        path = ".";
    
    int fd = (int)syscall(SYS_open, (long)path, O_RDONLY, 0, 0, 0);
    if (fd < 0) {
        printf("ls: cannot open %s\n", path);
        return -1;
    }
    
    char buf[ENTBUFSIZ];
    ssize_t n;
    for(struct posix_dent *e; (n = syscall(SYS_getdents, (long)fd, (long)buf, ENTBUFSIZ, 0, 0)) > 0; ) {
        for(ssize_t i = 0; i < n; i += e->d_reclen){
            e = (struct posix_dent*)&buf[i];
            bool hide = *e->d_name == '.';
            if(!hide || a){
                if (e->d_type == DT_DIR)
                    printf("%s/\n", e->d_name);
                else
                    printf("%s\n", e->d_name);
            }
            
        }
    }

    syscall(SYS_close, (long)fd, 0, 0, 0, 0);
    return 0;
}