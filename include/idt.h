#ifndef _IDT_H_
#define _IDT_H_

#include <stdint.h>

/* IDT Entry structure for IA-32 */
struct idt_entry {
    uint16_t isr_low;      /* Lower 16 bits of ISR's address */
    uint16_t kernel_cs;    /* Kernel code segment selector */
    uint8_t  reserved;     /* Set to zero */
    uint8_t  attributes;   /* Type and attributes (Flags) */
    uint16_t isr_high;     /* Higher 16 bits of ISR's address */
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;         /* 32-bit base address */
} __attribute__((packed));

void idt_init(void);
void set_idt_gate(int vector, void* isr, uint8_t flags);

#endif