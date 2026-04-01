#include "idt.h"
#include "gdt.h"
#include "io.h"
#include "stdio.h"
#include "sys/syscall.h"

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

extern void _syscall(void);
extern void isr_noerr(void);
extern void isr_err(void);

void fault_handler(uint32_t trapno, uint32_t err) {
    static const char *names[] = {
        "#DE", "#DB", "NMI", "#BP", "#OF", "#BR", "#UD", "#NM",
        "#DF", "??",  "#TS", "#NP", "#SS", "#GP", "#PF", "??",
        "#MF", "#AC", "#MC", "#XF"
    };
    const char *name = trapno < 20 ? names[trapno] : "??";
    printf("\nFAULT %s (int=0x%x) err=0x%x\n", name, trapno, err);
    printf("System halted.\n");
    __asm__ volatile ("cli; hlt");
}

void pic_remap(int offset1, int offset2) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, offset1);
    outb(0xA1, offset2);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void set_idt_gate(int vector, void* isr, uint8_t flags) {
    uintptr_t addr = (uintptr_t)isr;
    idt[vector].isr_low    = addr & 0xFFFF;
    idt[vector].isr_high   = (addr >> 16) & 0xFFFF;
    idt[vector].kernel_cs  = GDT_KERNEL_CODE;
    idt[vector].reserved   = 0;
    idt[vector].attributes = flags;
}

void idt_init(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    for (int i = 0; i < 256; i++)
        set_idt_gate(i, isr_noerr, 0x8E);

    /* exceptions with error codes */
    set_idt_gate(8,  isr_err, 0x8E); /* Double Fault       */
    set_idt_gate(10, isr_err, 0x8E); /* Invalid TSS        */
    set_idt_gate(11, isr_err, 0x8E); /* Segment Not Present*/
    set_idt_gate(12, isr_err, 0x8E); /* Stack Fault        */
    set_idt_gate(13, isr_err, 0x8E); /* General Protection */
    set_idt_gate(14, isr_err, 0x8E); /* Page Fault         */
    set_idt_gate(17, isr_err, 0x8E); /* Alignment Check    */

    /* syscall — DPL=3 */
    set_idt_gate(0x80, _syscall, 0xEE);

    __asm__ volatile ("lidt %0" : : "m"(idtp));
    pic_remap(0x20, 0x28);
    __asm__ volatile ("sti");
}