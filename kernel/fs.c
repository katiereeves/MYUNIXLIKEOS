#include "commands.h"
#include "vfs.h"
#include "string.h"
#include "pmm.h"
#include "stdio.h"
#include "elf.h"

vnode_t* current_dir = NULL;
vnode_t* vfs_root    = NULL;

static uint32_t vnodes_pool_used = 0;
static vnode_t  vnodes_pool[64];

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

void fs_init(void) {
    vnodes_pool_used = 0;
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

int k_mkdir(const char* path) {
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

int k_touch(const char* path) {
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

int k_install(const char* name, const uint8_t* data, uint32_t size) {
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

/* ── ELF execution ────────────────────────────────────────────────────────── */

#define USER_STACK_SIZE PAGE_SIZE

extern void jump_usermode(uint32_t entry, uint32_t user_stack);

/* k_exec(path, argv)
 *
 * argv is a NULL-terminated array of strings (may be NULL for no args).
 *
 * Stack layout seen by crt0 on entry to ring 3 (grows down, so built
 * from bottom up here):
 *
 *   user_esp   [ argc          ] top of built frame
 *              [ argv[0] ptr   ]
 *              [ argv[1] ptr   ]
 *              [ ...           ]
 *              [ NULL          ] end of argv ptr array
 *              [ "arg0\0"      ] string data
 *              [ "arg1\0"      ]
 *              [ ...           ]
 *
 * crt0 reads argc from [esp] and argv ptr from [esp+4].
 */
int k_exec(const char *path, const char **argv) {
    if (!path || !*path) { printf("exec: missing path\n"); return -1; }

    vnode_t *node = vfs_lookup(path);
    if (!node) {
        printf("exec: not found: %s\n", path);
        return -1;
    }
    if (node->flags != VFS_FILE) {
        printf("exec: not a file\n");
        return -1; }
    if (!node->content) {
        printf("exec: empty file\n");
        return -1;
    }

    uint32_t entry = elf_load((const uint8_t*)node->content, node->size);
    if (!entry) {
        printf("exec: ELF load failed\n");
        return -1;
    }

    /* allocate one page for the user stack */
    uint8_t *stack_page = pmm_alloc_page();
    if (!stack_page) {
        printf("exec: out of memory\n");
        return -1;
    }

    /* Build argc/argv on the user stack.
     * Use a pointer that walks DOWN from the top of the page.
     * We write string data first (at the high end), then the
     * pointer array + argc below that.
     */
    /* align down first so the frame we build is naturally aligned */
    uint8_t *stk_top = stack_page + USER_STACK_SIZE;
    uint8_t *str_ptr = (uint8_t *)((uint32_t)stk_top & ~0xFU);

    /* count args and copy string data onto stack */
    int argc = 0;
    uint32_t arg_ptrs[16];

    if (argv) {
        while (argv[argc] && argc < 15) {
            size_t len = strlen(argv[argc]) + 1;
            str_ptr -= len;
            memcpy(str_ptr, argv[argc], len);
            arg_ptrs[argc] = (uint32_t)str_ptr;
            argc++;
        }
    }

    /* Now build the pointer array + argc below the string data.
     * Layout (each cell is 4 bytes, stack grows down):
     *   argc
     *   argv[0]
     *   argv[1]
     *   ...
     *   argv[argc-1]
     *   NULL
     */
    uint32_t *frame = (uint32_t *)str_ptr;
    frame--;  *frame = 0;                          /* NULL terminator */
    for (int i = argc - 1; i >= 0; i--) {
        frame--;
        *frame = arg_ptrs[i];
    }
    frame--;  *frame = (uint32_t)argc;             /* argc */

    uint32_t user_esp = (uint32_t)frame;

    jump_usermode(entry, user_esp);

    return 0;
}