/* just to test fork */
#include "stdio.h"
#include "unistd.h"
#include "sched.h"

int main(void) {
    printf("before fork\n");
    pid_t pid = fork();

    if(pid == 0){
        for(int i = 0; i < 5; i++){
            printf("child: tick %d\n", i);
            sched_yield();
        }
        printf("child: done\n");
    }
    else{
        printf("parent: child pid=%d\n", pid);
        for(int i = 0; i < 5; i++){
            printf("parent: tick %d\n", i);
            sched_yield();
        }
        printf("parent: done\n");
    }
    return 0;
}