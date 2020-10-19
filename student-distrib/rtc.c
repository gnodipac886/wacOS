#include "rtc.h"
#include "lib.h"
#include "i8259.h"

/* rtc_init
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Initializes rtc and writes to the correct ports
 *		Side Effects: enables interrupt on PIC
 */
void __rtc_init__(){
    cli();
    /*set IRQ8 */
    outb(RTC_STATUS_REG+0xB, RTC_IO_PORT);  // Select RTC status register B (offset = 0xB)
    uint8_t reg_value = inb(CMOS_IO_PORT);  // Read register B value
    outb(RTC_STATUS_REG+0xB, RTC_IO_PORT);  // Select RTC status register B
    outb(reg_value | 0x40, CMOS_IO_PORT);   // Turn on Register B bit 6

    enable_irq(RTC_IRQ);                    // Enable interrupt for RTC on PIC
    sti();
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
	send_eoi(RTC_IRQ);

    /* Clear register C to allow another interrupt.*/
    outb(RTC_STATUS_REG+0x0C, RTC_IO_PORT);                 // Select RTC status register C
    inb(CMOS_IO_PORT);                                      // Dump the content

    //test_interrupts();

	sti();
}


