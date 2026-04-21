section .rodata
global cat_start, cat_end
cat_start: incbin "usr/obj/cat.elf"
cat_end:
global vi_start, vi_end
vi_start: incbin "usr/obj/vi.elf"
vi_end:
global ls_start, ls_end
ls_start: incbin "usr/obj/ls.elf"
ls_end:
global mkdir_start, mkdir_end
mkdir_start: incbin "usr/obj/mkdir.elf"
mkdir_end:
global segfault_start, segfault_end
segfault_start: incbin "usr/obj/segfault.elf"
segfault_end:
global forktest_start, forktest_end
forktest_start: incbin "usr/obj/forktest.elf"
forktest_end:
global sh_start, sh_end
sh_start: incbin "usr/obj/sh.elf"
sh_end:
