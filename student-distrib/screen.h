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

/* Mode X and general VGA parameters */
#if !defined(NDEBUG)
#define MEM_FENCE_WIDTH 256
#else
#define MEM_FENCE_WIDTH 0
#endif
#define VID_MEM_SIZE 			131072
#define MEM_FENCE_MAGIC 		0xF3
#define SCROLL_SIZE 			(SCREEN_X_DIM * SCREEN_Y_DIM)
#define SCREEN_SIZE 			(SCROLL_SIZE * 4 + 1)
#define BUILD_BUF_SIZE 			(SCREEN_SIZE + 20000)

#define NUM_SEQUENCER_REGS      5
#define NUM_GRAPHICS_REGS       9
#define NUM_CRTC_REGS          	25
#define NUM_ATTR_REGS			22
#define MODE_X_MEM_SIZE     	65536
#define VIDEO       			0xB8000
#define VGA_VIDEO       		0xA0000
#define VGA_SCREEN_END 			VGA_VIDEO + 0x3E7F 			// last 4 pixels of the VGA screen

#define FILE_MAX_LEN 			SCREEN_X_DIM * SCREEN_Y_DIM * 2

// mouse related:
#define VGA_CURSOR_SIZE 		6
#define FRAMES_PER_MOVE 		7 			// how many mouse frames per movement
#define BLACK_COL 				0
#define WHITE_COL				63
#define BLACK_PIX 				0x0000
#define WHITE_PIX 				0xFFFF

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
void play_loading_screen();
void draw_image_322(uint8_t * img);
void draw_image_565(pixel_565_t * img);
void draw_image_565_from_file(char * fname);
void get_cursor_image(pixel_565_t * buf);
void plot_cursor(int x, int y);
void save_cursor_background();

void set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS], unsigned char val);
void fill_palette_text();
void clear_mode_X();
void set_text_mode_3(int clear_scr);
void write_font_data();

#endif /* _SCREEN_H */
