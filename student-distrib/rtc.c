#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"

/* Flag to for rtc_read to determine if the next interrupt has occured */
int rtc_interrupt_occurred = 0;
int rtc_virtual_frequency = 2;				// default RTC interrupt frequency = 2 Hz

/* _rtc_init
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Initializes rtc and writes to the correct ports
 *		Side Effects: enables interrupt on PIC
 */
void __rtc_init__(){
	cli();
	/*set IRQ8 */
	outb(RTC_STATUS_REG + 0xB, RTC_IO_PORT);  // Select RTC status register B (offset = 0xB)
	uint8_t reg_value = inb(CMOS_IO_PORT);  // Read register B value
	outb(RTC_STATUS_REG + 0xB, RTC_IO_PORT);  // Select RTC status register B
	outb(reg_value | 0x40, CMOS_IO_PORT);   // Turn on Register B bit 6

	enable_irq(RTC_IRQ);					// Enable interrupt for RTC on PIC
	/* Virtualizing the RTC */
	int buf = 1024;							// RTC interrupt set to highest value = 1024 Hz.(Device interrupt rate will not change!)
	_rtc_write((void*)(&buf));				
	rtc_interrupt_occurred = 0;
	sti();
}

/* handle_rtc_interrupt 
 *	  Inputs: None
 *	  Return Value: None
 *	  Function: Cleans up interrupt after its been processed, then 
 *		  readies the RTC register for another interrupt.
 *	  Side Effects: none	 
 */
void handle_rtc_interrupt(){
	cli();
	send_eoi(RTC_IRQ);

	/* Clear register C to allow another interrupt.*/
	outb(RTC_STATUS_REG + 0x0C, RTC_IO_PORT);				 // Select RTC status register C
	inb(CMOS_IO_PORT);									 // Dump the content

	// test_interrupts();
	rtc_interrupt_occurred = 1;	  //flag for rtc_read
	sti();
}
/* _rtc_open
 *	  Inputs: None
 *	  Return Value: 1
 *	  Function: Reset rtc frequency to 2. 
 *	  Side Effects: none	 
 */
int _rtc_open(){
	/* int buf = 2;
	_rtc_write((void*)(&buf));						   // RTC interrupt default value = 2 Hz. */
	rtc_virtual_frequency = 2;						   // default RTC interrupt frequency = 2 Hz

	return 1;
}

/* _rtc_close
 *	  Inputs: None
 *	  Return Value: 1
 *	  Function: returns constant 1. 
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
	for(i = 0; i < (1024 / rtc_virtual_frequency); i++){

		while(rtc_interrupt_occurred == 0){
		
		}
		/* rtc interrupt has occured*/
		rtc_interrupt_occurred = 0; // Reset the flag
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
	/* if frequency is out of range 2-1024 (magic number)Hz, fail */
	if(freq > 1024 || freq < 2){
		return -1;
	}
	/* if freq is power of 2, set rtc to this frequency, else fail */
	if((freq & (freq - 1)) == 0){
		rtc_virtual_frequency = freq;
	}
	else{
		return -1;
	}

	/* This loop checks which power of 2 the frequency is. 
	// If frequency is not a power of 2, fail.
	uint32_t exponent = 0;	//Start from 2^0
	uint32_t pow_of_2 = 2;	//first power of 2 is 2 (ignore 1)
	while(freq>=pow_of_2){
		// freq is not a power of 2, fail.
		if(freq%pow_of_2 !=0){
			return -1;
		}
		pow_of_2*=2;		//next power of 2
		exponent++;			//increment exponent
	}

	// freq is a power of 2 and in range.
	outb(RTC_STATUS_REG+0x0A, RTC_IO_PORT);	 						//Index to RTC status register A
	uint8_t reg_value = inb(CMOS_IO_PORT);	  						//get initial register A value
	outb(RTC_STATUS_REG+0x0A, RTC_IO_PORT);	 						//Index to RTC status register A (again)
	// We're going to pass in rate as the lower 4 bits of register A and restore the other bits. 
	// frequency = 32768 >> (rate-1), so rate  = 16 - exponent 
	outb((reg_value & 0xF0) | (16-exponent) , CMOS_IO_PORT);		// 0xF0 for upper 4 bits, write register A.
	*/

	sti();
	return 0;
	
}
