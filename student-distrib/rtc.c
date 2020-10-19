#include "rtc.h"
#include "lib.h"
#include "i8259.h"

/* rtc_init 
 *      Inputs: None
 *      Return Value: None
 *      Function: Initiallizes RTC and enables PIC on port 8.
 *      Side Effects: none 
 */
void rtc_init(){

    /*set IRQ8 */
    outb(RTC_STATUS_REG+0xB, RTC_IO_PORT);  // Select RTC status register B (offset = 0xB)
    uint8_t reg_value = inb(CMOS_IO_PORT);  // Read register B value
    outb(RTC_STATUS_REG+0xB, RTC_IO_PORT);  // Select RTC status register B 
    outb(reg_value | 0x40, CMOS_IO_PORT);   // Turn on Register B bit 6

    enable_irq(RTC_IRQ);                    // Enable interrupt for RTC on PIC
}

/* handle_rtc_interrupt 
 *      Inputs: None
 *      Return Value: None
 *      Function: Cleans up interrupt after its been processed, then 
 *          readies the RTC register for another interrupt.
 *      Side Effects: none     
 */
void handle_rtc_interrupt(){
    cli();
	clear();
	printf("RTC Interrupt\n");

	send_eoi(RTC_IRQ);

    /* Clear register C to allow another interrupt.*/
    outb(0x0C, RTC_IO_PORT);                // Select RTC status register C
    inb(CMOS_IO_PORT);                      // Dump the content

	sti();
}


