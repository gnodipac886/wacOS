#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"
#include "scheduler.h"

#define MAX_TERMINALS   3

/* Flag to for rtc_read to determine if the next interrupt has occured */
int rtc_interrupt_occurred[MAX_TERMINALS] = {0, 0, 0};				// init to 0 for all three terminals
int _rtc_virtual_frequency[MAX_TERMINALS] = {VIRT_DEFAULT_FREQ, VIRT_DEFAULT_FREQ, VIRT_DEFAULT_FREQ};	

/* _rtc_init
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Initializes rtc and writes to the correct ports
 *		Side Effects: enables interrupt on PIC
 */
void __init_rtc__(){
	/*set IRQ8 */
	outb(RTC_STATUS_REG + 0xB, RTC_IO_PORT);  							// Select RTC status register B (offset = 0xB)
	uint8_t reg_value = inb(CMOS_IO_PORT);  							// Read register B value
	outb(RTC_STATUS_REG + 0xB, RTC_IO_PORT);  							// Select RTC status register B
	outb(reg_value | 0x40, CMOS_IO_PORT);   							// Turn on Register B bit 6

	enable_irq(RTC_IRQ);												// Enable interrupt for RTC on PIC
}

/* handle_rtc_interrupt
 *	  Inputs: None
 *	  Return Value: None
 *	  Function: Cleans up interrupt after its been processed, then
 *		  readies the RTC register for another interrupt.
 *	  Side Effects: none
 */
void handle_rtc_interrupt(){
	send_eoi(RTC_IRQ);

	/* Clear rtc register C to allow another interrupt.*/
	outb(RTC_STATUS_REG + 0x0C, RTC_IO_PORT);				 			// Select RTC status register C
	inb(CMOS_IO_PORT);										 			// Dump the content

	// test_interrupts();
	rtc_interrupt_occurred[get_curr_scheduled()] = 1;	  				//flag for rtc_read
	sti();
}
/* _rtc_open
 *	  Inputs: None
 *	  Return Value: 1
 *	  Function: Reset rtc frequency to 2.
 *	  Side Effects: none
 */
int _rtc_open(){
	int buf = VIRT_DEFAULT_FREQ;										// default RTC interrupt frequency = 2 Hz
	_rtc_write((void*)(&buf));						   
	return 1;
}

/* _rtc_close
 *	  Inputs: None
 *	  Return Value: 0
 *	  Function: returns constant 0.
 *	  Side Effects: none
 */
int _rtc_close(){
	return 0;
}

/* rtc_read
 *	  Inputs: Reads data from rtc
 *	  Return Value: Always zero
 *	  Function: Read RTC, and after interrupt occurs, return 0.
 *	  Side Effects: none
 */
int _rtc_read(){
	/* Wait until next rtc interrupt occurs.
	 * RTC Virtualization: returns after the desired frequency met.
	 * Ex: 512 Hz virtual frequency, for every 1024/512 = 2 interrupts,
	 * 		_rtc_read returns, generating a 512 Hz rate.
	 */
	int i;
	for(i = 0; i < (DEVICE_MAX_FREQ / _rtc_virtual_frequency[get_curr_scheduled()]); i+=3){

		while(rtc_interrupt_occurred[get_curr_scheduled()] == 0);		// checks for interrupts

		/* rtc interrupt has occured*/
		rtc_interrupt_occurred[get_curr_scheduled()] = 0; 				// Reset the flag
	}
	return 0;
}

/* rtc_write
 *	  Inputs: 4 byte integer that specifies rtc interrupt rate
 *	  Return Value: -1 on failure, else other;
 *	  Function: Set rtc frequency. Range: 1-1024 Hz
 *	  Side Effects: none
 */
int _rtc_write(void* buf){
	cli();
	int freq = *((int*)buf);
	/* if frequency is out of range 2-1024 Hz, fail */
	if(freq > DEVICE_MAX_FREQ || freq < VIRT_DEFAULT_FREQ){
		return -1;
	}
	/* if freq is power of 2, set rtc to this frequency, else fail */
	if((freq & (freq - 1)) == 0){
		_rtc_virtual_frequency[get_curr_scheduled()] = freq;
	}
	else{
		return -1;
	}

	sti();
	return 0;

}
