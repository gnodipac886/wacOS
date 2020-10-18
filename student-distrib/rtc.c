#include "rtc.h"
#include "lib.h"

/* Initialize RTC */
void rtc_init(){

    /*set IRQ8 */
    outb(RTC_STATUS_REG+0xB, RTC_IO_PORT);  // Select RTC status register B (offset = 0xB)
    uint8_t reg_value = inb(CMOS_IO_PORT);  // Read register B value
    outb(RTC_STATUS_REG+0xB, RTC_IO_PORT);  // Select RTC status register B 
    outb(reg_value | 0x40, CMOS_IO_PORT);   // Turn on Register B bit 6

}

/* Future implementatiion */
/* void rtc_irq_handler(){

} */

/* void rtc_freq_set(uint32_t){

} */