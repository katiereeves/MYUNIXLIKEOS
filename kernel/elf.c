#include "elf.h"
#include "string.h"
#include "stdio.h"
#include "pmm.h"

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

extern void jump_usermode(uint32_t entry, uint32_t user_stack);

int elf_exec(const uint8_t *data, size_t size) {
    uint32_t entry = elf_load(data, size);
    if (entry == 0) {
        printf("elf: load failed\n");
        return -1;
    }

    uint8_t *stack_page = (uint8_t *)pmm_alloc_page();
    if (!stack_page) {
        printf("elf: out of memory for user stack\n");
        return -1;
    }

    uint32_t user_esp = (uint32_t)(stack_page + PAGE_SIZE);

    jump_usermode(entry, user_esp);
    return 0;
}