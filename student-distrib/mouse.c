#include "lib.h"
#include "i8259.h"
#include "mouse.h"
#include "screen.h"
#include "paging.h"
#include "gui.h"

#define SCREEN_X    	80
#define SCREEN_Y    	25
#define VGA_SCREEN_X 	320
#define VGA_SCREEN_Y 	200
#define VIDEO       	0xB8000
#define VGA_VIDEO		0xA0000
#define ATTRIB      	0x7
#define MS_COLOR 		0xE0 		// yellow
#define MS_SPD 			15 			// higher the slower, 22 good

ms_packet_t packet;
uint8_t 	state; 					// there are 3 states for each byte of interrupt

int32_t left_flag = 0;
int32_t drag_flag = 0;
int32_t curr_x;
int32_t curr_y;

int32_t screen_x_max;
int32_t screen_y_max;

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

	if(GUI_ACTIVATE){
		screen_x_max = VGA_SCREEN_X;
		screen_y_max = VGA_SCREEN_Y;
		curr_x = screen_x_max / 2;
		curr_y = screen_y_max / 2;
	}
	else{
		screen_x_max = SCREEN_X;
		screen_y_max = SCREEN_Y;
		curr_x = screen_x_max / 2;
		curr_y = screen_y_max / 2;
		prev_char = *(uint8_t *)(video_mem + ((screen_x_max * curr_y + curr_x) << 1));
		prev_color= *(uint8_t *)(video_mem + ((screen_y_max * curr_y + curr_x) << 1) + 1);
	}

	state = 0;
}

/* handle_mouse_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: reads mouse and tracks pointer
 *		Side Effects: none
 */
void handle_mouse_interrupt(){
	cli();
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

			
			// printf("%d, %d\n", curr_x, curr_y);
			if(packet.l_btn){
				// printf("Left Button\n");
				if(left_flag == 1){
					drag_flag = 1;
				}else{
					left_flag = 1;
				}
			}
			else {
				left_flag = 0;
				drag_flag = 0;
			}
			if(packet.r_btn){
				// printf("Right Button\n");
			}
			if(packet.m_btn){
				// printf("Middle Button\n");
			}
			
			update_mouse_cursor();

			state = 0;
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

void update_mouse_cursor(){
	int32_t scale = 1;
	int32_t sum = 0;
	int32_t dx, dy, sx, sy;

	dx = packet.x_sign ? NEGATIVE_NUM | packet.x_move : packet.x_move;
	dx = !(packet.x_move & ~7) ? 0 : dx;				// reject all deltas under and including 7

	dy = packet.y_sign ? NEGATIVE_NUM | packet.y_move : packet.y_move;
	dy = !(packet.y_move & ~7) ? 0 : dy;				// reject all deltas under and including 7

	sx = packet.x_sign ? -1 : 1;
	sx = dx == 0 ? 0 : sx;

	sy = packet.y_sign ? -1 : 1;
	sy = dy == 0 ? 0 : sy;

	sum = dx + dy < 0 ? -(dx + dy) : dx + dy;

	asm volatile(
		"bsr 	%1, 		%%eax;"						// move page directory address into cr3
		"movl 	%%eax, 		%0;"						// move the scale into the variable scale
		:"=r"(scale)									// outputs
		:"r"(sum)										// inputs
		:"%eax" 	
	);

	// printf("%d, %d\n", (int8_t)packet.x_move, (int8_t)packet.y_move);
	// printf("before: %d, %d scale: %d ", dx, dy, scale);
	scale = (MS_SPD - scale) >> 1;
	dx /= scale == 0 ? 1 : scale;			// configure the scaling for movement	
	dy /= scale == 0 ? 1 : scale;
	// printf("after: %d, %d, scale: %d\n", dx, dy, scale);

	if(GUI_ACTIVATE){
		if (drag_flag == 1){
			change_window_location(curr_x, curr_y, dx, dy, scale, sx, sy);
		}
		draw_mouse_cursor(&curr_x, &curr_y, dx, dy, scale, sx, sy);
	}
	else{
		clear_prev_cursor();
		curr_x += dx;
		curr_y -= dy;

		curr_x = curr_x >= SCREEN_X ? SCREEN_X - 1 : curr_x;
		curr_y = curr_y >= SCREEN_Y ? SCREEN_Y - 1 : curr_y;

		curr_x = curr_x < 0 ? 0 : curr_x;
		curr_y = curr_y < 0 ? 0 : curr_y;
		draw_curr_cursor();
	}
}

void clear_prev_cursor(){
	if(GUI_ACTIVATE){
		draw_rectangle(curr_x, curr_y, 0, VGA_CURSOR_SIZE, VGA_CURSOR_SIZE);
	}
	else{
		// revert the previous character on the screen
		*(uint8_t *)(video_mem + ((screen_x_max * curr_y + curr_x) << 1)) = prev_char;
		*(uint8_t *)(video_mem + ((screen_x_max * curr_y + curr_x) << 1) + 1) = prev_color;
	}
}

void draw_curr_cursor(){
	if(GUI_ACTIVATE){
		draw_rectangle(curr_x, curr_y, 3, VGA_CURSOR_SIZE, VGA_CURSOR_SIZE);
	}
	else{
		// set cursor character
		prev_char = *(uint8_t *)(video_mem + ((screen_x_max * curr_y + curr_x) << 1));
		prev_color= *(uint8_t *)(video_mem + ((screen_x_max * curr_y + curr_x) << 1) + 1);
		*(uint8_t *)(video_mem + ((screen_x_max * curr_y + curr_x) << 1)) = ' ';
		*(uint8_t *)(video_mem + ((screen_x_max * curr_y + curr_x) << 1) + 1) = MS_COLOR;
	}
}
