[BITS 32]
global _start
extern main

_start:
    xor ebp, ebp            ; mark outermost frame

    mov eax, [esp]          ; argc
    lea ecx, [esp+4]        ; argv = &stack[1]  (ptr to first char* ptr)

    ; push args for main(argc, argv) in cdecl order (right to left)
    push ecx                ; argv
    push eax                ; argc
    call main
    add esp, 8

    ; exit(return value)
    mov ebx, eax            ; exit code
    mov eax, 0x24           ; SYS_exit
    int 0x80

.hang:
    cli
    hlt
    jmp .hang