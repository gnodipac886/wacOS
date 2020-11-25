#ifndef _SCREEN_H
#define _SCREEN_H

#include "types.h"

#define SCREEN_X_DIM			320							/* pixels; must be divisible by 4             */
#define SCREEN_Y_DIM			200							/* pixels                                     */
#define SCREEN_DIM 				SCREEN_X_DIM * SCREEN_Y_DIM

#define NUM_PLANE 				4
#define PLANE_X_DIM	 			SCREEN_X_DIM / NUM_PLANE
#define PLANE_Y_DIM				SCREEN_Y_DIM
#define PLANE_DIM 				PLANE_X_DIM * PLANE_Y_DIM

#define NUM_GRAPHICS_REGS       9
#define NUM_CRTC_REGS          	25
#define NUM_ATTR_REGS			22
#define MODE_X_MEM_SIZE     	65536
#define VIDEO       			0xB8000
#define VGA_VIDEO       		0xA0000
#define VGA_SCREEN_END 			VGA_VIDEO + 0x3E7F 			// last 4 pixels of the VGA screen

// mouse related:
#define VGA_CURSOR_SIZE 4
#define FRAMES_PER_MOVE 7 			// how many mouse frames per movement

// pixel related
typedef struct pixel_565{
	union{
		uint16_t pixel;
		struct{
			uint16_t b 	: 5;
			uint16_t g 	: 6;
			uint16_t r 	: 5;
		};
	};
} pixel_565_t;

// pixel related
typedef struct pixel_888{
	uint8_t b;
	uint8_t g;
	uint8_t r;
} pixel_888_t;

void __screen_init__();
void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]);
void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS]);
void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]);
void VGA_blank (int blank_bit);
void fill_palette_mode_x_basic();
void fill_palette_mode_x_332();
void fill_palette_mode_x_custom(uint8_t * palette);
void clear_screens();

void draw_rectangle(int screen_x, int screen_y, uint8_t color, int rect_x, int rect_y);
void draw_circle(int x, int y, int r, int color);
void plot_pixel(int x, int y, uint8_t color);
uint8_t get_pixel(int x, int y);
void show_screen();

void draw_mouse_cursor(int * curr_x, int * curr_y, int dx, int dy, int frames, int sx, int sy);
void draw_image_322(uint8_t * img);
void draw_image_565(pixel_565_t * img);

#endif /* _SCREEN_H */
