/* user.ld — linker script for flat-physical no-paging kernel userspace */

ENTRY(_start)

/* Explicit program headers */
PHDRS {
    text PT_LOAD FLAGS(5);   /* R+X */
    data PT_LOAD FLAGS(6);   /* R+W */
}

SECTIONS {
    /* Start .text @ 0x08049000. */
    . = 0x08049000;

    .text   : { *(.text .text.*)   } :text
    .rodata : { *(.rodata .rodata.*) } :text

    . = ALIGN(0x1000);

    .data   : { *(.data .data.*) } :data
    .bss    : {
        __bss_start = .;
        *(.bss .bss.* COMMON)
        __bss_end = .;
    } :data
}
