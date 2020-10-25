#include "rtc.h"
#include "lib.h"
#include "i8259.h"
/* Flag to for rtc_read to determine if the next interrupt has occured */
volatile int rtc_interrupt_occurred = 0;

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
    _rtc_write(2);                           // RTC interrupt default value = 2 Hz.
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

    // test_interrupts();

    rtc_interrupt_occurred =1;      //flag for rtc_read
	sti();
}

void rtc_open(){

}

void rtc_close(){

}

/* rtc_read
 *      Inputs: Reads data from rtc
 *      Return Value: Always zero
 *      Function: Read RTC, and after interrupt occurs, return 0. 
 *      Side Effects: none     
 */
int _rtc_read(){
    /* Wait until next rtc interrupt occurs */
    while(rtc_interrupt_occurred == 0){
    
    }
    /* rtc interrupt has occured*/
    rtc_interrupt_occurred = 0; // Reset the flag
    return 0;
}

/* rtc_write
 *      Inputs: 4 byte integer that specifies rtc interrupt rate
 *      Return Value: -1 on failure, else other;
 *      Function: Set rtc frequency. Range: 1-1024 Hz 
 *      Side Effects: none     
 */
int _rtc_write(int freq){
    cli();
    /* if frequency is out of range 1-1024 Hz, fail */
    if(freq>1024 || freq<1){
        return -1;
    }

    /* if frequency is not a power of 2, fail.*/
    uint32_t exponent = -1;
    uint32_t pow_of_2 = 1;
    while(freq>=pow_of_2){
        /* freq is not a power of 2, fail.*/
        if(freq%pow_of_2 !=0){
            return -1;
        }
        pow_of_2*=2;
        exponent++;    
    }

    /* freq is a power of 2 and in range.*/
    outb(RTC_STATUS_REG+0x0A, RTC_IO_PORT);     //Index to RTC status register A
    uint8_t reg_value = inb(CMOS_IO_PORT);      //get initial register A value
    outb(RTC_STATUS_REG+0x0A, RTC_IO_PORT);     //Index to RTC status register A (again)
    /* We're going to pass in rate as the lower 4 bits of register A and restore the other bits. 
     * frequency = 32768 >> (rate-1), so rate  = 16 - exponent*/ 
    outb((reg_value & 0xF0) | (16-exponent) , CMOS_IO_PORT);    // write register A.
    sti();
    return 0;
}
