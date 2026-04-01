#include "gdt.h"

#define GDT_ENTRIES 6   /* null, kernel code, kernel data, user code, user data, tss */

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr   gdtp;
static struct tss       tss_entry;

extern void gdt_load(struct gdt_ptr*);
extern void tss_load(void);

static void set_gdt_gate(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].base_low    = base & 0xFFFF;
    gdt[i].base_middle = (base >> 16) & 0xFF;
    gdt[i].base_high   = (base >> 24) & 0xFF;
    gdt[i].limit_low   = limit & 0xFFFF;
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access      = access;
}

static void set_tss(int i, uint32_t base, uint32_t limit) {
    set_gdt_gate(i, base, limit, 0x89, 0x00);
}

void tss_set_kernel_stack(uint32_t stack) {
    tss_entry.esp0 = stack;
}

void gdt_init(void) {
    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base  = (uint32_t)&gdt;

    /* gdt stuff */
    set_gdt_gate(0, 0, 0,          0x00, 0x00); /* null */
    set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* kernel code */
    set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* kernel data */
    set_gdt_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* user code (DPL=3) */
    set_gdt_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* user data (DPL=3) */

    /* tss stuff */
    for (uint32_t i = 0; i < sizeof(tss_entry); i++)
        ((uint8_t*)&tss_entry)[i] = 0;
    tss_entry.ss0  = GDT_KERNEL_DATA;
    tss_entry.esp0 = 0; /* set properly before entering user mode */
    tss_entry.iopb_offset = sizeof(tss_entry);

    set_tss(5, (uint32_t)&tss_entry, sizeof(tss_entry) - 1);

    gdt_load(&gdtp);
    tss_load();
}