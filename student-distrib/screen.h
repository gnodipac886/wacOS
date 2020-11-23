#ifndef _SCREEN_H
#define _SCREEN_H

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

void __screen_init__();
void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]);
void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS]);
void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]);
void VGA_blank (int blank_bit);
void fill_palette_mode_x();
void clear_screens();

void draw_rectangle(int screen_x, int screen_y, uint8_t color, int rect_x, int rect_y);
void draw_circle(int x, int y, int r, int color);
void plot_pixel(int x, int y, uint8_t color);
void show_screen();

void draw_mouse_cursor(int * curr_x, int * curr_y, int dx, int dy, int frames, int sx, int sy);
void draw_image(uint8_t * img);

#endif /* _SCREEN_H */
