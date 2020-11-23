#ifndef _MOUSE_H
#define _MOUSE_H

#include "types.h"

#define MS_PORT 		0x60 			// IO port for mouse
#define PS2_PORT 		0x64 			// PS2 io command port
#define MS_IRQ 			12 				// mouse interrupt

#define GET_STATUS_BYTE 0x20 			// Get Compaq Status Byte
#define SET_STATUS_BYTE 0x60
#define ENABLE_MOUSE 	0xF4			// enable the mouse devices
#define ENABLE_ACK 		0xA8
#define MS_CMD_BYTE 	0xD4 			// send to 0x64 before sending bytes to 0x64
#define MS_DEFAULT 		0xF6
#define STATUS_INIT_1 	~0x20 			// Disable Mouse Clock set to 0
#define STATUS_INIT_2 	0x02 			// enable IRQ 12

#define MSK0 			0x01
#define MSK1 			0x02
#define MSK2 			0x04
#define MSK3 			0x08
#define MSK4 			0x10
#define MSK5 			0x20
#define MSK6 			0x40
#define MSK7 			0x80

#define NEGATIVE_NUM 	0xFFFFFF00

typedef struct ms_packet{
	union{
		uint8_t byte0;
		struct{
			uint8_t l_btn 	: 1;		// left button
			uint8_t r_btn 	: 1;		// right button
			uint8_t m_btn 	: 1;		// middle button
			uint8_t one 	: 1;		// always 1
			uint8_t x_sign 	: 1;		// x sign bit
			uint8_t y_sign 	: 1;		// y sign bit
			uint8_t x_over 	: 1;		// x overflow bit
			uint8_t y_over 	: 1;		// y overflow bit
		};
	};
	uint8_t x_move;						// x movement
	uint8_t y_move;						// y movement
	union{
		uint8_t byte3;
		struct{
			uint8_t scroll 	: 4; 		// scroll bits
			uint8_t btn_4 	: 1;		// 4th button
			uint8_t btn_5 	: 1;		// 5th button
			uint8_t zero 	: 2;		// zero
		};
	};
} ms_packet_t;

void __mouse_init__();
void handle_mouse_interrupt();
void mouse_wait(int type);

// helper functions
void update_mouse_cursor();
void clear_prev_cursor();
void draw_curr_cursor();

#endif /* _MOUSE_H */
