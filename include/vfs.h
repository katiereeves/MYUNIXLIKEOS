#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include "sys/types.h"

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_CHAR_DEVICE 0x03
#define VFS_MAX_FDS     0x100

struct vnode;

typedef struct vfs_entry {
    char name[128];
    uint64_t inode;
    uint32_t type;
    struct vnode* node;
} vfs_entry_t;

typedef struct vnode {
    char name[32];
    uint32_t flags;
    struct vnode* parent;
    struct vnode* children[16];
    uint32_t child_count;
    uint32_t size;
    char* content;
} vnode_t;

extern vnode_t* vfs_root;
extern vnode_t* current_dir;

void vfs_init(void);

int      vfs_mkdir(const char* path);
int      vfs_create(const char* path);
vnode_t* vfs_lookup(const char* path);
int      vfs_register_file(const char* name, const uint8_t* data, uint32_t size);

int   vfs_fd_offset(int fd);
int   vfs_fd_size(int fd);
void* vfs_fd_content(int fd);

/* existing vfs functions */
void     vfs_init(void);
vnode_t* vfs_lookup(const char* path);
int      vfs_mkdir(const char* path);
int      vfs_create(const char* path);
int vfs_open(const char* path, int flags);
int      vfs_close(int fd);
ssize_t  vfs_read(int fd, void* buf, size_t nbyte);
ssize_t  vfs_write(int fd, const void* buf, size_t nbyte);
int vfs_fd_set_offset(int fd, uint32_t offset);
ssize_t vfs_getdents(int fd, void *buf, size_t nbyte);

#endif