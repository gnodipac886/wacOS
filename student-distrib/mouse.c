#include "lib.h"
#include "i8259.h"
#include "mouse.h"

#define SCREEN_X    80
#define SCREEN_Y    25

uint32_t prev_x = SCREEN_X;
uint32_t prev_y = SCREEN_Y;

uint32_t curr_x;
uint32_t curr_y;

/* __mouse_init__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initialization of the mouse for interrupt
 *		Side Effects: none
 */
void __mouse_init__(){
	uint8_t status_byte;

	mouse_wait(1); 												// wait for the input to clear
	outb(0xA8, PS2_PORT); 										// get the status byte to show up
	outb(GET_STATUS_BYTE, PS2_PORT); 							// get the status byte to show up
	status_byte = inb(PS2_PORT); 								// get the status byte
	status_byte = (status_byte & STATUS_INIT_1) | STATUS_INIT_2;// set the status bytes to init

	outb(SET_STATUS_BYTE, PS2_PORT); 							// Set Compaq Status
	outb(status_byte, PS2_PORT);								// write the updates status byte to ps2 controller

	enable_irq(MS_IRQ);

	curr_x = prev_x;
	curr_y = prev_y;
}

/* handle_mouse_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: reads mouse and tracks pointer
 *		Side Effects: none
 */
void handle_mouse_interrupt(){
	printf("INT_MOUSE!\n");
}

/* mouse_wait
 * 		Inputs: type - 0 - output buffer status 	(inb)	must be set to continue
 					 - 1 - input buffer status 		(outb) 	must be cleared to continue
 * 		Return Value: none
 * 		Function: waits for buffer to clear on 0x64
 *		Side Effects: none
 */
void mouse_wait(int type){
	while((inb(PS2_PORT) & (1 << type)) != ((~type) & 1));
}
