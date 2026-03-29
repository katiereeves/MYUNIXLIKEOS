#include "stdint.h"
#include "nvram.h"
#include "sys/types.h"

/* Motorola MC146818AP driver, standard pc nvram */

void readNVRAM(uint8_t *out) {
   uint8_t *outptr = out;
   for (uint8_t i = 0; i < 128; i++) {
      __asm__ volatile(
          "cli\n\t"              /* Clear interrupts */
          "movb %1, %%al\n\t"    /* Move index to al */
          "outb %%al, $0x70\n\t" /* Write index to NVRAM address port */
          "inb $0x71, %%al\n\t"  /* Read 1 byte from NVRAM data port */
          "movb %%al, %0\n\t"    /* Store result */
          "sti\n\t"              /* Enable interrupts */
          : "=m"(*outptr++)      /* output */
          : "r"(i)               /* input */
          : "%al", "memory"      /* clobbers */
      );
   }
}

void writeNVRAM(uint8_t *in) {
   uint8_t *inptr = in;
   for (uint8_t i = 0; i < 128; i++) {
      __asm__ volatile(
          "cli\n\t"              /* Clear interrupts */
          "movb %0, %%al\n\t"    /* Move index to al */
          "outb %%al, $0x70\n\t" /* Write index to NVRAM address port */
          "movb %1, %%al\n\t"    /* Move value to al */
          "outb %%al, $0x71\n\t" /* Write 1 byte to NVRAM data port */
          "sti\n\t"              /* Enable interrupts */
          :                      /* no output */
          : "r"(i),              /* input 0 */
            "r"(*inptr++)        /* input 1 */
          : "%al", "memory"      /* clobbers */
      );
   }
}

uint8_t bcd(uint8_t v) {
   return (v & 0x0F) + ((v >> 4) * 10);
}