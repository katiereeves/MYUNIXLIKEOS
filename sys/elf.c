#include "elf.h"
#include "string.h"
#include "stdio.h"
#include "pmm.h"
#include "vfs.h"

int elf_validate(const uint8_t *data, size_t size) {
    if (size < sizeof(Elf32_Ehdr)) {
        printf("elf: too small\n");
        return -1;
    }

    const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)data;

    if (*((uint32_t *)ehdr->e_ident) != ELF_MAGIC) {
        printf("elf: bad magic\n");
        return -1;
    }
    if (ehdr->e_type != ET_EXEC) {
        printf("elf: not an executable\n");
        return -1;
    }
    if (ehdr->e_machine != EM_386) {
        printf("elf: not IA-32\n");
        return -1;
    }
    if (ehdr->e_phoff == 0 || ehdr->e_phnum == 0) {
        printf("elf: no program headers\n");
        return -1;
    }

    return 0;
}

uint32_t elf_load(const uint8_t *data, size_t size) {
    if (elf_validate(data, size) != 0)
        return 0;

    const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)data;

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        const Elf32_Phdr *phdr = (const Elf32_Phdr *)(
            data + ehdr->e_phoff + i * ehdr->e_phentsize
        );

        if (phdr->p_type != PT_LOAD)
            continue;

        /* bounds check: segment must lie within the file buffer */
        if ((uint64_t)phdr->p_offset + phdr->p_filesz > size) {
            printf("elf: segment %u out of bounds\n", i);
            return 0;
        }

        uint8_t *dest = (uint8_t *)phdr->p_vaddr;

        /* copy initialised data from file */
        memcpy(dest, data + phdr->p_offset, phdr->p_filesz);
        /* zero-fill BSS region (memsz > filesz) */
        if (phdr->p_memsz > phdr->p_filesz)
            memset(dest + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
    }

    return ehdr->e_entry;
}

#define USER_STACK_SIZE PAGE_SIZE

extern void jump_usermode(uint32_t entry, uint32_t user_stack);

int elf_exec(const char *path, const char **argv) {
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

    /* Build argc/argv on the user stack. */
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