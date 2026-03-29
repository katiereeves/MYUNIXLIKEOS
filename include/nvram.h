#ifndef _SYS_NVRAM_H_
#define _SYS_NVRAM_H_

#include "stdint.h"

/* CMOS/NVRAM table for MC146818A RTC compatible chipsets.
 * Lots of legacy stuff in here, so I feel like the comments may help who ever
 * needs to read this.
 *
 * special thanks to:
 * https://web.archive.org/web/20240119203005/http://www.bioscentral.com/misc/cmosmap.htm
 */

struct nvram_t {
    /* RTC time and alarm registers (offsets 00h - 09h) */

    uint8_t rtc_sec;         /* Current seconds (00-59)                   */
    uint8_t rtc_sec_alarm;   /* Seconds alarm value                       */
    uint8_t rtc_min;         /* Current minutes (00-59)                   */
    uint8_t rtc_min_alarm;   /* Minutes alarm value                       */
    uint8_t rtc_hour;        /* Current hours; format set by reg B bit 1  */
    uint8_t rtc_hour_alarm;  /* Hours alarm value                         */
    uint8_t rtc_weekday;     /* Day of week (1 = Sunday ... 7 = Saturday) */
    uint8_t rtc_day;         /* Day of month (01-31)                      */
    uint8_t rtc_month;       /* Month (01-12)                             */
    uint8_t rtc_year;        /* Year within century (00-99)               */

    /* RTC status / control registers (offsets 0Ah - 0Dh) */

    uint8_t status_reg_a;
    /* Bit 7    :  Update-in-progress flag (read-only):
     *               0 = Registers are stable and safe to read
     *               1 = Time update cycle is in progress, do not read time
     * Bits 6-4 :  Oscillator / divider select:
     *               010 = 32.768 KHz (normal operation)
     * Bits 3-0 :  Periodic interrupt rate select:
     *               0000 = None
     *               0110 = 1.024 KHz  (976.562 us period)
     *               0111 = 512 Hz     (1.953 ms period)
     *               1111 = 2 Hz       (500 ms period)
     */

    uint8_t status_reg_b;
    /* Bit 7 : Update cycle control:
     *               0 = Allow normal periodic updates
     *               1 = Halt update; registers may be safely written
     * Bit 6 : Periodic interrupt enable:
     *               0 = Disabled (default)
     *               1 = IRQ fires at rate set in reg A bits 3-0
     * Bit 5 : Alarm interrupt enable:
     *               0 = Disabled (default)
     *               1 = IRQ fires when time matches alarm registers
     * Bit 4 : Update-ended interrupt enable:
     *               0 = Disabled (default)
     *               1 = IRQ fires after each completed update cycle
     * Bit 3 : Square-wave output enable:
     *               0 = Disabled (default)
     *               1 = SQW pin toggles at rate set in reg A bits 3-0
     * Bit 2 : Data format select:
     *               0 = BCD encoding (default)
     *               1 = Binary (pure integer) encoding
     * Bit 1 : Hour format select:
     *               0 = 24-hour mode (default)
     *               1 = 12-hour mode (AM/PM)
     * Bit 0 : Daylight saving time adjust enable:
     *               0 = Disabled (default)
     *               1 = Add one hour at start of DST, subtract at end
     */

    uint8_t status_reg_c;
    /* Read-only interrupt flag register; cleared by reading this register.
     *
     * Bit 7    : IRQF — master interrupt request flag:
     *               1 = At least one unmasked interrupt source is pending
     * Bit 6    : PF — periodic interrupt pending
     * Bit 5    : AF — alarm interrupt pending
     * Bit 4    : UF — update-ended interrupt pending
     * Bits 3-0 : Reserved (always 0)
     */

    uint8_t status_reg_d;
    /* Read-only battery / RAM validity register.
     *
     * Bit 7 : VRT — valid RAM and time flag:
     *               0 = CMOS battery has failed; data is unreliable
     *               1 = Battery is good; CMOS contents are valid
     * Bits 6-0 : Reserved (always 0)
     */

    /* POST / diagnostic status (offsets 0Eh - 0Fh) */

    uint8_t status_diag;
    /* Diagnostic status byte written by POST.
     *
     * Bit 7 : RTC power loss flag:
     *               0 = CMOS has maintained power since last boot
     *               1 = CMOS lost power; time and config may be invalid
     * Bit 6 : CMOS checksum status:
     *               0 = Checksum over 10h-2Dh is valid
     *               1 = Checksum mismatch detected
     * Bit 5 : Configuration validity:
     *               0 = Equipment configuration matches CMOS
     *               1 = Configuration record is corrupt or inconsistent
     * Bit 4 : Memory size mismatch:
     *               0 = Detected RAM matches CMOS memory count
     *               1 = Detected RAM differs from stored count
     * Bit 3 : Fixed disk / adapter init result:
     *               0 = Initialised successfully
     *               1 = Initialisation failed
     * Bit 2 : RTC time validity:
     *               0 = Time is valid
     *               1 = Time is invalid or was never set
     * Bits 1-0 : Reserved
     */

    uint8_t shutdown_stat;
    /* Shutdown status code written by POST/BIOS before a soft reset.
     * Used to resume execution at the correct re-entry point.
     *
     *   00h = Power-on or hard reset
     *   01h = Memory size determination complete
     *   02h = Memory test passed
     *   03h = Memory test failed
     *   04h = POST complete — hand off to boot loader
     *   05h = JMP via double-word pointer (with EOI)
     *   06h = Protected-mode internal tests passed
     *   07h = Protected-mode internal tests failed
     *   08h = Memory sizing failed
     *   09h = INT 15h block-move shutdown request
     *   0Ah = JMP via double-word pointer (without EOI)
     *   0Bh = Reserved for 80386 chipset use
     */

    /* Drive type and system configuration (offsets 10h - 12h) */

    uint8_t fdd_types;
    /* Installed floppy drive types, one nibble per drive.
     *
     * Bits 7-4 : Drive 0 type
     * Bits 3-0 : Drive 1 type
     *
     *   0000 = Not installed
     *   0001 = 360 KB  (5.25")
     *   0010 = 1.2 MB  (5.25")
     *   0011 = 720 KB  (3.5")
     *   0100 = 1.44 MB (3.5")
     *   0101 = 2.88 MB (3.5")
     */

    uint8_t sys_cnfg_setting;
    /* System configuration flags.
     *
     * Bit 7 : PS/2 mouse support:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 6 : Extended memory test (above 1 MB):
     *               0 = Enabled
     *               1 = Disabled
     * Bit 5 : Memory-test tick/beep sound:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 4 : Memory parity error checking:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 3 : Setup utility hot-key prompt at POST:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 2 : Hard disk type-47 parameter storage location:
     *               0 = 0000:0300h
     *               1 = Upper 1 KB of conventional memory
     * Bit 1 : Pause on POST error / wait for F1:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 0 : NumLock state on boot:
     *               0 = Off
     *               1 = On
     */

    uint8_t hdd_types;
    /* Installed hard disk types, one nibble per drive.
     *
     * Bits 7-4 : Hard disk 0 type
     * Bits 3-0 : Hard disk 1 type
     *
     *   0000 = No drive installed
     *   0001 = Type 1
     *   ...
     *   1110 = Type 14
     *   1111 = Extended type (16-47) - see hdd_type_ext at offset 19h
     */

    uint8_t typematic_perms;
    /* Keyboard typematic (auto-repeat) configuration.
     *
     * Bit 7    : Typematic rate programming:
     *               0 = Use BIOS default rate
     *               1 = Use values in bits 6-2
     * Bits 6-5 : Repeat delay before first repeat character:
     *               00 = 250 ms
     *               01 = 500 ms
     *               10 = 750 ms
     *               11 = 1000 ms
     * Bits 4-2 : Repeat rate (characters per second):
     *               000 = 30 cps
     *               111 = 2 cps
     * Bits 1-0 : Reserved
     */

    uint8_t installed_dev;
    /* Installed devices flags (mirrors INT 11h equipment word, low byte).
     *
     * Bits 7-6 : Number of floppy drives (valid only if bit 0 is set):
     *               00 = 1 drive
     *               01 = 2 drives
     * Bits 5-4 : Initial video mode:
     *               00 = Defer to display adapter BIOS
     *               01 = CGA 40-column colour
     *               10 = CGA 80-column colour
     *               11 = Monochrome Display Adapter (MDA)
     * Bit 3    : Reserved (some BIOSes: display adapter presence)
     * Bit 2    : Keyboard installed:
     *               0 = Not present
     *               1 = Present
     * Bit 1    : Math coprocessor (x87 FPU) installed:
     *               0 = Not present
     *               1 = Present
     * Bit 0    : Floppy drive(s) installed:
     *               0 = No floppy
     *               1 = One or more floppy drives present
     */

    /* Base and extended memory counts (offsets 15h - 18h) */

    uint8_t least_sig_byte_base;  /* Base memory size LSB (in KB, max 640) */
    uint8_t most_sig_byte_base;   /* Base memory size MSB                  */
    uint8_t least_sig_byte_ext;   /* Extended memory size LSB (in KB)      */
    uint8_t most_sig_byte_ext;    /* Extended memory size MSB              */

    /* Hard disk extended type and parameter tables (offsets 19h - 2Bh) */

    uint8_t hdd_type_ext[2];  /* Extended HDD type codes for drives C and D
                               * (used when hdd_types nibble == 0Fh)        */
    uint8_t hdd_c_data[9];    /* Drive C parameter table (cylinders, heads,
                               * sectors, write-precomp, etc.)              */
    uint8_t hdd_d_data[9];    /* Drive D parameter table (same layout)      */

    /* System operational flags (offset 2Dh) */

    uint8_t sys_op_flags;
    /* Bit 7 : Weitek math coprocessor present:
     *               0 = Absent
     *               1 = Present
     * Bit 6 : Floppy drive seek on boot:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 5 : Boot drive sequence:
     *               0 = Drive C first
     *               1 = Drive A first
     * Bit 4 : Boot-time CPU speed:
     *               0 = High speed
     *               1 = Low speed
     * Bit 3 : External (L2) cache:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 2 : Internal (L1) cache:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 1 : Fast Gate A20 switching:
     *               0 = Enabled
     *               1 = Disabled
     * Bit 0 : Turbo switch function:
     *               0 = Enabled
     *               1 = Disabled
     */

    /* Checksum and actual extended memory (offsets 2Eh - 31h) */

    uint8_t cksum[2];                   /* Two's complement checksum of bytes
                                         * 10h through 2Dh (MSB first)        */
    uint8_t least_sig_byte_ext_actual;  /* Actual extended memory LSB (in KB) */
    uint8_t most_sig_byte_ext_actual;   /* Actual extended memory MSB         */

    /* POST flags and BIOS options (offsets 32h - 36h) */

    uint8_t century_BCD;    /* Century in BCD (e.g., 0x20 = year 20xx) */

    uint8_t post_flags;
    /* Bit 7    : BIOS ROM size:
     *               0 = 64 KB
     *               1 = 128 KB
     * Bits 6-1 : Reserved
     * Bit 0    : POST cache test result:
     *               0 = Failed
     *               1 = Passed
     */

    uint8_t bios_opt_flags[2];
    /* Byte 0 (offset 34h) — security and adapter ROM shadowing:
     *
     * Bit 7 : Boot sector virus protection:
     *               0 = Disabled
     *               1 = Enabled
     * Bit 6 : BIOS password checking:
     *               0 = Disabled
     *               1 = Enabled
     * Bit 5 : Adapter ROM shadow C800h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 4 : Adapter ROM shadow CC00h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 3 : Adapter ROM shadow D000h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 2 : Adapter ROM shadow D400h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 1 : Adapter ROM shadow D800h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 0 : Adapter ROM shadow DC00h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     *
     * Byte 1 (offset 35h) — upper adapter / system ROM shadowing:
     *
     * Bit 7 : Adapter ROM shadow E000h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 6 : Adapter ROM shadow E400h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 5 : Adapter ROM shadow E800h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 4 : Adapter ROM shadow EC00h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 3 : System ROM shadow F000h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 2 : Video ROM shadow C000h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 1 : Video ROM shadow C400h (16 KB):
     *               0 = Disabled
     *               1 = Enabled
     * Bit 0 : Numeric coprocessor POST test:
     *               0 = Disabled
     *               1 = Enabled
     */

    uint8_t chipst_info; /* Chipset info (non-standard, vendor-defined) */

    uint8_t pswrd_and_colour;
    /* Bits 7-4 : Password seed — do not modify
     * Bits 3-0 : Setup screen colour palette index:
     *               07h = White on black
     *               70h = Black on white
     *               17h = White on blue
     *               20h = Black on green
     *               30h = Black on cyan
     *               47h = White on red
     *               57h = White on magenta
     *               60h = Black on brown
     */

    uint8_t is_pswrd_crypt; /* Non-zero if the stored password is encrypted */

    /* Extended checksum, ID fields, and century (offsets 38h - 3Fh) */

    uint8_t cksum_ext[2];  /* Checksum covering the extended CMOS region          */
    uint8_t model_n;       /* System model number identifier                      */
    uint8_t serial_n[6];   /* System serial number (ASCII or BCD, vendor-defined) */
    uint8_t crc;           /* CRC of the serial number / ID fields                */
    uint8_t century;       /* Century byte (binary) - paired with rtc_year        */

    /* Extended RTC and RAM control (offsets 40h onward, chipset-specific) */

    uint8_t rtc_date_alarm;    /* Date-of-month alarm register (RTC extension) */
    uint8_t cntrl_reg_ext[2];  /* Extended RTC control registers               */
    uint8_t _reserved[2];
    uint8_t rtc_clck_addr[2];  /* RTC clock address / index registers          */
    uint8_t ram_ext_addr[2];   /* Extended CMOS RAM address port registers     */
    uint8_t __reserved;
    uint8_t ram_data_port;     /* Extended CMOS RAM data port                  */
    uint8_t ___reserved[10];
} __attribute__((packed));

void readNVRAM(uint8_t *out);
void writeNVRAM(uint8_t *in);
uint8_t bcd(uint8_t v);

#endif /* sys/nvram.h */