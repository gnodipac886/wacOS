#include "pit.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"
#include "scheduler.h"

/* _pit_init
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Initializes PIT, set PIT counter frequency to 40hz, or 25 millisecond time intervals. 
 *		Side Effects: Enables interrupt on PIC IRQ0
 */
void __init_pit__(){
	// cli();
	/* set PIT to mode to: Rate generater/real time counter */
    uint16_t count = PIT_DEF_FREQ / PIT_FREQ ;      // calculate divisor to send as command word
    outb(PIT_CMD_BYTE, PIT_CMD_REG);                // Send command byte to command register
    outb(count & 0xFF, PIT_DATA_REG_0);             // Send lower 8 bytes first
    outb(count >> 8, PIT_DATA_REG_0);               // Send the upper 8 bytes

	// enable_irq(PIT_IRQ);					        // Enable interrupt for PIT on PIC

	// sti();
}

/* handle_pit_interrupt 
 *	  Inputs: None
 *	  Return Value: None
 *	  Function: Sends EOI, and calls schedule. Schedules the tasks.
 *	  Side Effects: none	 
 */
void handle_pit_interrupt(){
    // cli();
    int* curr_pid = _get_pid_tracker();
    // printf("PITINT %d\n", get_curr_scheduled());

    if(get_curr_scheduled() == -1){
        switch_process(0, 1);
    }

    switch_process(curr_pid[get_curr_scheduled()], curr_pid[(get_curr_scheduled() + 1) % MAX_TERMINALS]);       // switch between terminals and run their proces
    
    send_eoi(PIT_IRQ);
    sti();

}






