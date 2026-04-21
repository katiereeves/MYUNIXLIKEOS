/* TODO:
 - Move more things out of here.
*/
#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "vfs.h"
#include "page.h"
#include "pmm.h"
#include "io.h"
#include "string.h"
#include "stdio.h"
#include "sys/syscall.h"
#include "idt.h"
#include "gdt.h"
#include "sys/time.h"
#include "unistd.h"
#include "sys/proc/proc.h"
#include "sys/sched/sched.h"
#include "multiboot.h"

static unsigned char _stdout_buf[2048];
static unsigned char _stderr_buf[256];
static unsigned char _stdin_buf[256];

static FILE _stdin_f = {
    .flags  = 0,
    .fileno = 0,
    .buf    = _stdin_buf,
    .bptr   = _stdin_buf,
    .len    = sizeof(_stdin_buf),
    .free   = 0,
    .iobf   = _IOLBF,
    .eof    = 0,
};

static FILE _stdout_f = {
    .flags  = 1,
    .fileno = 1,
    .buf    = _stdout_buf,
    .bptr   = _stdout_buf,
    .len    = sizeof(_stdout_buf),
    .free   = 0,
    .iobf   = _IOLBF,
    .eof    = 0,
};

static FILE _stderr_f = {
    .flags  = 1,
    .fileno = 2,
    .buf    = _stderr_buf,
    .bptr   = _stderr_buf,
    .len    = sizeof(_stderr_buf),
    .free   = 0,
    .iobf   = _IONBF,
    .eof    = 0,
};

FILE *stdout = &_stdout_f;
FILE *stderr = &_stderr_f;
FILE *stdin  = &_stdin_f;

extern uint32_t stack_top;
extern void install_user_progs(void);
extern void irq0_handler(void);
extern struct proc *run_queue_head;
extern void jump_usermode(uint32_t entry, uint32_t user_stack);

void kernel_main(uint32_t magic, uint32_t mb_addr) {
    __asm__ volatile("cli");

    if (magic != 0x36D76289) {
        __asm__ volatile("cli; hlt");
    }

    void *mmap_tag = NULL;
    struct multiboot_tag *tag =
        (struct multiboot_tag *)(mb_addr + 8);

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            mmap_tag = tag;
            break;
        }
        tag = (struct multiboot_tag *)
            ((uint8_t *)tag + ((tag->size + 7) & ~7u));
    }

    pmm_init(mmap_tag);
    gdt_init();
        
    idt_init();
    page_init();
    set_idt_gate(32, irq0_handler, 0x8E);

    tss_set_kernel_stack((uint32_t)&stack_top);
    vfs_init();
    install_user_progs();

    printf("*nix IA-32 Kernel booted\n");

    time_t t;
    time(&t);
    struct tm bt;
    const char *days[]   = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    localtime_r(&t, &bt);
    printf("%s %s %i:%i %i UTC\n", days[bt.tm_wday],
        months[bt.tm_mon], bt.tm_hour, bt.tm_min, bt.tm_year);

    struct proc *sh = proc_from_elf("sh");

    sched_enqueue(sh);
    current_proc          = run_queue_head;
    current_proc->p_state = PROC_RUNNING;

    switch_page_dir(current_proc->p_page_dir);
    tss_set_kernel_stack(current_proc->p_kernel_stack);

    sched_init(1000);

    outb(0x21, 0xFE);

    jump_usermode(current_proc->p_ctx.eip, current_proc->p_ctx.useresp);
}