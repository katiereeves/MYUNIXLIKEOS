[global syscall_interrupt_handler]
[extern syscall_handler]

syscall_interrupt_handler:
    pushad          ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI (Save Context)

    ; Push arguments for: void syscall_handler(int num, int a1, int a2, int a3)
    push edx        ; Argument 3
    push ecx        ; Argument 2
    push ebx        ; Argument 1
    push eax        ; Syscall Number
    
    call syscall_handler
    
    add esp, 16     ; Clean up the 4 arguments we pushed
    popad           ; Restore all registers (Give the app its brain back)
    iretd           ; 32-bit Interrupt Return (Note the 'd')