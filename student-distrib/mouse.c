#include "lib.h"
#include "i8259.h"
#include "mouse.h"

#define SCREEN_X    80
#define SCREEN_Y    25
#define VIDEO       0xB8000
#define ATTRIB      0x7
#define MS_COLOR 	0xE0 		// yellow
#define MS_SPD 		22 			// higher the slower

ms_packet_t packet;
uint8_t 	state; 		// there are 3 states for each byte of interrupt

int32_t curr_x;
int32_t curr_y;

static char* video_mem = (char *)VIDEO;
static char prev_char;
static char prev_color;

/* __mouse_init__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initialization of the mouse for interrupt
 *		Side Effects: none
 */
void __mouse_init__(){
	uint8_t status_byte;

	mouse_wait(1); 												// wait for the input to clear
	// outb(ENABLE_ACK, PS2_PORT); 								// get the status byte to show up
	outb(GET_STATUS_BYTE, PS2_PORT); 							// get the status byte to show up
	status_byte = inb(MS_PORT); 								// get the status byte
	status_byte = (status_byte & STATUS_INIT_1) | STATUS_INIT_2;// set the status bytes to init

	outb(SET_STATUS_BYTE, PS2_PORT); 							// Set Compaq Status
	outb(status_byte, MS_PORT);									// write the updates status byte to ps2 controller

	// outb(MS_CMD_BYTE, PS2_PORT);								// tell command port we are writing to mouse
	// outb(MS_DEFAULT, MS_PORT); 									// enable the mouse
	// inb(MS_PORT); 												// ack

	outb(MS_CMD_BYTE, PS2_PORT);								// tell command port we are writing to mouse
	outb(ENABLE_MOUSE, MS_PORT); 								// enable the mouse
	// inb(MS_PORT); 												// ack
	
	enable_irq(MS_IRQ);

	curr_x = SCREEN_X / 2;
	curr_y = SCREEN_Y / 2;
	prev_char = *(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1));
	prev_color= *(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1) + 1);

	state = 0;
}

/* handle_mouse_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: reads mouse and tracks pointer
 *		Side Effects: none
 */
void handle_mouse_interrupt(){
	int32_t scale = 1;
	int32_t sum = 0;
	int32_t dx, dy;
	switch(state){
		case 0:
			packet.byte0 = inb(MS_PORT);
			state++;
			break;

		case 1:
			packet.x_move = inb(MS_PORT);
			state++;
			break;

		case 2:
			packet.y_move = inb(MS_PORT);

			// revert the previous character on the screen
			*(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1)) = prev_char;
			*(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1) + 1) = prev_color;

			dx = packet.x_sign ? NEGATIVE_NUM | packet.x_move : packet.x_move;
			dy = packet.y_sign ? NEGATIVE_NUM | packet.y_move : packet.y_move;

			sum = dx + dy < 0 ? -(dx + dy) : dx + dy;

			asm volatile(
				"bsr 	%1, 		%%eax;"						// move page directory address into cr3
				"movl 	%%eax, 		%0;"						// move the scale into the variable scale
				:"=r"(scale)									// outputs
				:"r"(sum)										// inputs
				:"%eax" 	
			);

			// printf("before: %d, %d scale: %d ", dx, dy, scale);
			scale = (MS_SPD - scale) >> 1;
			dx /= scale == 0 ? 1 : scale;			// configure the scaling for movement	
			dy /= scale == 0 ? 1 : scale;
			// printf("after: %d, %d, scale: %d\n", dx, dy, scale);

			curr_x += dx;
			curr_y -= dy;

			curr_x = curr_x >= SCREEN_X ? SCREEN_X - 1 : curr_x;
			curr_y = curr_y >= SCREEN_Y ? SCREEN_Y - 1 : curr_y;

			curr_x = curr_x < 0 ? 0 : curr_x;
			curr_y = curr_y < 0 ? 0 : curr_y;

			// set cursor character
			prev_char = *(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1));
			prev_color= *(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1) + 1);
			*(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1)) = ' ';
			*(uint8_t *)(video_mem + ((SCREEN_X * curr_y + curr_x) << 1) + 1) = MS_COLOR;

			state = 0;
			// printf("%d, %d\n", curr_x, curr_y);
			if(packet.l_btn){
				// printf("Left Button\n");
			}
			if(packet.r_btn){
				// printf("Right Button\n");
			}
			if(packet.m_btn){
				// printf("Middle Button\n");
			}
			break;

		default:
			break;
	}

	sti();
	send_eoi(MS_IRQ);
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
