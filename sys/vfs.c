#include "vfs.h"
#include "string.h"
#include "pmm.h"
#include "stdio.h"
#include "elf.h"
#include "sys/types.h"
#include "fcntl.h"
#include "dirent.h"

vnode_t* current_dir = NULL;
vnode_t* vfs_root    = NULL;

static uint32_t vnodes_pool_used = 0;
static vnode_t  vnodes_pool[64];

static struct {
    vnode_t* vnode;
    uint32_t offset;
    int      flags;
    int      in_use;
} fd_table[VFS_MAX_FDS];

static int alloc_fd(void) {
    for (int i = 3; i < VFS_MAX_FDS; i++) {
        if (!fd_table[i].in_use) return i;
    }
    return -1;
}

static vnode_t* alloc_vnode(void) {
    if (vnodes_pool_used >= 64) return NULL;
    vnode_t* n = &vnodes_pool[vnodes_pool_used++];
    n->name[0]     = '\0';
    n->flags       = 0;
    n->parent      = NULL;
    n->child_count = 0;
    n->content     = NULL;
    for (int i = 0; i < 16; i++) n->children[i] = NULL;
    return n;
}

void vfs_init(void) {
    vnodes_pool_used = 0;

    for (int i = 0; i < VFS_MAX_FDS; i++) {
        fd_table[i].vnode  = NULL;
        fd_table[i].offset = 0;
        fd_table[i].flags  = 0;
        fd_table[i].in_use = 0;
    }

    vfs_root = alloc_vnode();
    if (!vfs_root) return;
    strcpy(vfs_root->name, "/");
    vfs_root->flags       = VFS_DIRECTORY;
    vfs_root->parent      = NULL;
    vfs_root->child_count = 0;
    vfs_root->content     = NULL;
    current_dir = vfs_root;
}

vnode_t* vfs_lookup(const char* path) {
    if (!path || !*path) return NULL;
    if (strcmp(path, "/")  == 0) return vfs_root;
    if (strcmp(path, ".")  == 0) return current_dir;
    if (strcmp(path, "..") == 0)
        return current_dir->parent ? current_dir->parent : current_dir;

    const char* name = (*path == '/') ? path + 1 : path;
    for (uint32_t i = 0; i < current_dir->child_count; i++) {
        vnode_t* c = current_dir->children[i];
        if (strcmp(c->name, name) == 0) return c;
    }
    return NULL;
}

int vfs_mkdir(const char* path) {
    if (!path || !*path) { printf("mkdir: missing name\n"); return -1; }
    if (!current_dir) return -1;
    if (current_dir->child_count >= 16) { printf("mkdir: directory full\n"); return -1; }
    if (vfs_lookup(path)) { printf("mkdir: already exists\n"); return -1; }
    vnode_t* node = alloc_vnode();
    if (!node) { printf("mkdir: out of nodes\n"); return -1; }
    strcpy(node->name, path);
    node->flags       = VFS_DIRECTORY;
    node->parent      = current_dir;
    node->child_count = 0;
    node->content     = NULL;
    current_dir->children[current_dir->child_count++] = node;
    return 0;
}

int vfs_create(const char* path) {
    if (!path || !*path) { printf("touch: missing name\n"); return -1; }
    if (!current_dir) return -1;
    if (current_dir->child_count >= 16) { printf("touch: directory full\n"); return -1; }
    if (vfs_lookup(path)) { printf("touch: already exists\n"); return -1; }
    vnode_t* node = alloc_vnode();
    if (!node) { printf("touch: out of nodes\n"); return -1; }
    strcpy(node->name, path);
    node->flags       = VFS_FILE;
    node->parent      = current_dir;
    node->child_count = 0;
    node->content     = (char*)pmm_alloc_z(64);
    if (node->content) node->content[0] = '\0';
    current_dir->children[current_dir->child_count++] = node;
    return 0;
}

int vfs_open(const char* path, int flags) {
    if (!path || !*path) return -1;

    vnode_t* node = vfs_lookup(path);

    if (!node) {
        if (!(flags & O_CREAT)) return -1;
        if (current_dir->child_count >= 16) return -1;
        node = alloc_vnode();
        if (!node) return -1;
        strcpy(node->name, path);
        node->flags   = VFS_FILE;
        node->parent  = current_dir;
        node->size    = 0;
        node->content = (char*)pmm_alloc_z(4096);
        if (node->content) node->content[0] = '\0';
        current_dir->children[current_dir->child_count++] = node;
    }

    if (flags & O_TRUNC) {
        if (node->content) node->content[0] = '\0';
        node->size = 0;
    }

    /* allow directories to be opened read-only, reject writes */
    if ((node->flags & VFS_DIRECTORY) && (flags & (O_WRONLY | O_RDWR)))
        return -1;

    int fd = alloc_fd();
    if (fd < 0) return -1;

    fd_table[fd].vnode  = node;
    fd_table[fd].offset = (flags & O_APPEND) ? node->size : 0;
    fd_table[fd].flags  = flags;
    fd_table[fd].in_use = 1;

    return fd;
}

int vfs_register_file(const char* name, const uint8_t* data, uint32_t size) {
    if (!name || !data || !size) return -1;
    if (vfs_root->child_count >= 16) return -1;
    vnode_t* node = alloc_vnode();
    if (!node) return -1;
    strcpy(node->name, name);
    node->flags   = VFS_FILE;
    node->parent  = vfs_root;
    node->content = (char*)data;
    node->size    = size;
    vfs_root->children[vfs_root->child_count++] = node;
    return 0;
}

int vfs_fd_offset(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return -1;
    return (int)fd_table[fd].offset;
}

int vfs_fd_size(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return -1;
    if (!fd_table[fd].vnode) return -1;
    return (int)fd_table[fd].vnode->size;
}

void* vfs_fd_content(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return NULL;
    if (!fd_table[fd].vnode) return NULL;
    return fd_table[fd].vnode->content;
}

ssize_t vfs_read(int fd, void* buf, size_t nbyte) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return -1;
    vnode_t* v = fd_table[fd].vnode;
    if (!v || !v->content) return -1;

    size_t avail = v->size > fd_table[fd].offset
                 ? v->size - fd_table[fd].offset : 0;
    size_t n = nbyte < avail ? nbyte : avail;
    memcpy(buf, v->content + fd_table[fd].offset, n);
    fd_table[fd].offset += n;
    return (ssize_t)n;
}

ssize_t vfs_write(int fd, const void* buf, size_t nbyte) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return -1;
    vnode_t* v = fd_table[fd].vnode;
    if (!v || !v->content) return -1;
    if (fd_table[fd].flags & O_RDONLY) return -1;

    /* 4096-byte buffer */
    size_t avail = 4096 > fd_table[fd].offset ? 4096 - fd_table[fd].offset : 0;
    size_t n = nbyte < avail ? nbyte : avail;
    memcpy(v->content + fd_table[fd].offset, buf, n);
    fd_table[fd].offset += n;
    if (fd_table[fd].offset > v->size) v->size = fd_table[fd].offset;
    return (ssize_t)n;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return -1;
    fd_table[fd].in_use = 0;
    fd_table[fd].vnode  = NULL;
    return 0;
}

int vfs_fd_set_offset(int fd, uint32_t offset) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return -1;
    fd_table[fd].offset = offset;
    return 0;
}

ssize_t vfs_getdents(int fd, void *buf, size_t nbyte) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) return -1;
    vnode_t *dir = fd_table[fd].vnode;
    if (!dir || !(dir->flags & VFS_DIRECTORY)) return -1;
    if (nbyte < sizeof(struct posix_dent)) return -1;

    struct posix_dent *d      = (struct posix_dent *)buf;
    uint32_t           off    = fd_table[fd].offset;
    size_t             written = 0;

    /* virtual entries: 0 = ".", 1 = ".." */
    if (off == 0 && written + sizeof(struct posix_dent) <= nbyte) {
        d->d_ino    = (uint32_t)(dir - vnodes_pool);
        d->d_reclen = sizeof(struct posix_dent);
        d->d_type   = DT_DIR;
        strcpy(d->d_name, ".");
        d++;
        written += sizeof(struct posix_dent);
        off++;
    }

    if (off == 1 && written + sizeof(struct posix_dent) <= nbyte) {
        vnode_t *parent = dir->parent ? dir->parent : dir;
        d->d_ino    = (uint32_t)(parent - vnodes_pool);
        d->d_reclen = sizeof(struct posix_dent);
        d->d_type   = DT_DIR;
        strcpy(d->d_name, "..");
        d++;
        written += sizeof(struct posix_dent);
        off++;
    }

    // TODO: fix this
    /* real children — offset 2+ maps to children[off-2] */
    while ((off - 2) < dir->child_count &&
           written + sizeof(struct posix_dent) <= nbyte) {
        vnode_t *child = dir->children[off - 2];
        d->d_ino    = (uint32_t)(child - vnodes_pool);
        d->d_reclen = sizeof(struct posix_dent);
        d->d_type   = (child->flags & VFS_DIRECTORY) ? DT_DIR : DT_REG;
        strcpy(d->d_name, child->name);
        d++; written += sizeof(struct posix_dent); off++;
    }

    fd_table[fd].offset = off;
    return (ssize_t)written;
}