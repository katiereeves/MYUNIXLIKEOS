#include "vfs.h"
#include <stdint.h>
extern uint8_t cat_start[], cat_end[];
extern uint8_t vi_start[], vi_end[];
extern uint8_t ls_start[], ls_end[];
extern uint8_t mkdir_start[], mkdir_end[];
extern uint8_t segfault_start[], segfault_end[];
extern uint8_t forktest_start[], forktest_end[];
extern uint8_t sh_start[], sh_end[];
void install_user_progs(void) {
    vfs_register_file("cat", cat_start, (uint32_t)(cat_end - cat_start));
    vfs_register_file("vi", vi_start, (uint32_t)(vi_end - vi_start));
    vfs_register_file("ls", ls_start, (uint32_t)(ls_end - ls_start));
    vfs_register_file("mkdir", mkdir_start, (uint32_t)(mkdir_end - mkdir_start));
    vfs_register_file("segfault", segfault_start, (uint32_t)(segfault_end - segfault_start));
    vfs_register_file("forktest", forktest_start, (uint32_t)(forktest_end - forktest_start));
    vfs_register_file("sh", sh_start, (uint32_t)(sh_end - sh_start));
}
