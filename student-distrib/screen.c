#include "lib.h"
#include "screen.h"
#include "paging.h"
#include "octree.h"
#include "filesystem.h"
#include "text.h"
// #include "../images/big_sur.h"
// #include "../images/bar.h"

static unsigned short mode_X_seq[NUM_SEQUENCER_REGS] = {
    0x0100, 0x2101, 0x0F02, 0x0003, 0x0604
};

static unsigned short mode_X_CRTC[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5404, 0x8005, 0xBF06, 0x1F07,
    0x0008, 0x4109, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x0014, 0x9615, 0xB916, 0xE317,
    0xFF18
};

static unsigned char mode_X_attr[NUM_ATTR_REGS * 2] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
    0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
    0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
    0x10, 0x41, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x00,
    0x14, 0x00, 0x15, 0x00
};

static unsigned short mode_X_graphics[NUM_GRAPHICS_REGS] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x4005, 0x0506, 0x0F07,
    0xFF08
};

/* VGA register settings for text mode 3 (color text) */
static unsigned short text_seq[NUM_SEQUENCER_REGS] = {
    0x0100, 0x2001, 0x0302, 0x0003, 0x0204
};

static unsigned short text_CRTC[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5504, 0x8105, 0xBF06, 0x1F07,
    0x0008, 0x4F09, 0x0D0A, 0x0E0B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x1F14, 0x9615, 0xB916, 0xA317,
    0xFF18
};

static unsigned char text_attr[NUM_ATTR_REGS * 2] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
    0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
    0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
    0x10, 0x0C, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x08,
    0x14, 0x00, 0x15, 0x00
};

static unsigned short text_graphics[NUM_GRAPHICS_REGS] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x1005, 0x0E06, 0x0007,
    0xFF08
};

unsigned char* mem_image = (unsigned char*)VGA_VIDEO;                     /* pointer to start of video memory */
unsigned char build[BUILD_BUF_SIZE + 2 * MEM_FENCE_WIDTH];

uint32_t fb_addr = (uint32_t)VGA_VIDEO; 			// address of current frame buffer we are using (0 based, 0xA0000 = 0x0)
uint8_t build_buf[320 * 200];
uint8_t screen_buf[320 * 200];

uint8_t cursor_save1[VGA_CURSOR_SIZE * VGA_CURSOR_SIZE];
uint8_t cursor_save2[VGA_CURSOR_SIZE * VGA_CURSOR_SIZE];
pixel_565_t cursor_img[VGA_CURSOR_SIZE * VGA_CURSOR_SIZE];

// int32_t curr_mouse_x = SCREEN_X_DIM / 2;
// int32_t curr_mouse_y = SCREEN_Y_DIM / 2;

int32_t fb1_mouse_x_prev = SCREEN_X_DIM / 2;
int32_t fb1_mouse_y_prev = SCREEN_Y_DIM / 2;
int32_t fb2_mouse_x_prev = SCREEN_X_DIM / 2;
int32_t fb2_mouse_y_prev = SCREEN_Y_DIM / 2;

/*
 * __screen_init__
 *   DESCRIPTION: call initialize functions for the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: goes into vga mode
 */
void __screen_init__(){
	int save_x, save_y;
	VGA_blank (1);                               	/* blank the screen      */
	set_seq_regs_and_reset (mode_X_seq, 0x63);   	/* sequencer registers   */
	set_CRTC_registers (mode_X_CRTC);            	/* CRT control registers */
	set_attr_registers (mode_X_attr);            	/* attribute registers   */
	set_graphics_registers (mode_X_graphics);    	/* graphics registers    */
	fill_palette_mode_x_basic();			 		/* palette colors        */
	clear_screens();								/* set video memory      */
	VGA_blank(0);			         				/* unblank the screen    */

	// draw_rectangle(100, 50, 3, 100, 100);
	// draw_circle(160, 100, 100, 0xE0);
	// draw_image_322((uint8_t*)bar_map);

	// draw_image_565((pixel_565_t*)((void*)big_sur_map));
	// while(1)
	get_cursor_image(cursor_img);
	draw_image_565_from_file("big_sur.bin");

	// save the middle mouse frame
	for(save_y = 0; save_y < VGA_CURSOR_SIZE; save_y++){
		for(save_x = 0; save_x < VGA_CURSOR_SIZE; save_x++){
			cursor_save1[save_y * VGA_CURSOR_SIZE + save_x] = get_pixel(fb1_mouse_x_prev + save_x, fb1_mouse_y_prev + save_y);
		}
	}
}

/*
 * set_graphics_registers
 *   DESCRIPTION: Set VGA graphics registers.
 *   INPUTS: table -- table of graphics register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]){
    REP_OUTSW (0x03CE, table, NUM_GRAPHICS_REGS);
}

/*
 * set_CRTC_registers
 *   DESCRIPTION: Set VGA cathode ray tube controller (CRTC) registers.
 *   INPUTS: table -- table of CRTC register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS]){
    /* clear protection bit to enable write access to first few registers */
    outw (0x0011, 0x03D4);
    REP_OUTSW (0x03D4, table, NUM_CRTC_REGS);
}

/*
 * set_attr_registers
 *   DESCRIPTION: Set VGA attribute registers.  Attribute registers use
 *                a single port and are thus written as a sequence of bytes
 *                rather than a sequence of words.
 *   INPUTS: table -- table of attribute register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]){
    /* Reset attribute register to write index next rather than data. */
    asm volatile (
	"inb (%%dx),%%al"
      : : "d" (0x03DA) : "eax", "memory");
    REP_OUTSB (0x03C0, table, NUM_ATTR_REGS * 2);
}

/*
 * VGA_blank
 *   DESCRIPTION: Blank or unblank the VGA display.
 *   INPUTS: blank_bit -- set to 1 to blank, 0 to unblank
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void VGA_blank(int blank_bit){
    /*
     * Move blanking bit into position for VGA sequencer register
     * (index 1).
     */
    blank_bit = ((blank_bit & 1) << 5);

    asm volatile (
	"movb $0x01,%%al         /* Set sequencer index to 1. */       ;"
	"movw $0x03C4,%%dx                                             ;"
	"outb %%al,(%%dx)                                              ;"
	"incw %%dx                                                     ;"
	"inb (%%dx),%%al         /* Read old value.           */       ;"
	"andb $0xDF,%%al         /* Calculate new value.      */       ;"
	"orl %0,%%eax                                                  ;"
	"outb %%al,(%%dx)        /* Write new value.          */       ;"
	"movw $0x03DA,%%dx       /* Enable display (0x20->P[0x3C0]) */ ;"
	"inb (%%dx),%%al         /* Set attr reg state to index. */    ;"
	"movw $0x03C0,%%dx       /* Write index 0x20 to enable. */     ;"
	"movb $0x20,%%al                                               ;"
	"outb %%al,(%%dx)                                               "
      : : "g" (blank_bit) : "eax", "edx", "memory");
}

/*
 * fill_palette_text
 *   DESCRIPTION: Fill VGA palette with default VGA colors.
 *                Only the first 32 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 32 palette colors
 */   
void fill_palette_text(){
    /* 6-bit RGB (red, green, blue) values VGA colors and grey scale */
    static unsigned char palette_RGB[32][3] = {
	{0x00, 0x00, 0x00}, {0x00, 0x00, 0x2A},   /* palette 0x00 - 0x0F    */
	{0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x2A},   /* basic VGA colors       */
	{0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x2A},
	{0x2A, 0x15, 0x00}, {0x2A, 0x2A, 0x2A},
	{0x15, 0x15, 0x15}, {0x15, 0x15, 0x3F},
	{0x15, 0x3F, 0x15}, {0x15, 0x3F, 0x3F},
	{0x3F, 0x15, 0x15}, {0x3F, 0x15, 0x3F},
	{0x3F, 0x3F, 0x15}, {0x3F, 0x3F, 0x3F},
	{0x00, 0x00, 0x00}, {0x05, 0x05, 0x05},   /* palette 0x10 - 0x1F    */
	{0x08, 0x08, 0x08}, {0x0B, 0x0B, 0x0B},   /* VGA grey scale         */
	{0x0E, 0x0E, 0x0E}, {0x11, 0x11, 0x11},
	{0x14, 0x14, 0x14}, {0x18, 0x18, 0x18},
	{0x1C, 0x1C, 0x1C}, {0x20, 0x20, 0x20},
	{0x24, 0x24, 0x24}, {0x28, 0x28, 0x28},
	{0x2D, 0x2D, 0x2D}, {0x32, 0x32, 0x32},
	{0x38, 0x38, 0x38}, {0x3F, 0x3F, 0x3F}
    };

    /* Start writing at color 0. */
    outb (0x00, 0x03C8);

    /* Write all 32 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB, 32 * 3);
}

/*
 * fill_palette_mode_x_basic
 *   DESCRIPTION: Fill VGA palette with necessary colors for the adventure
 *                game.  Only the first 64 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 64 palette colors
 */
void fill_palette_mode_x_basic(){
	    /* 6-bit RGB (red, green, blue) values for first 64 colors */
    /* these are coded for 2 bits red, 2 bits green, 2 bits blue */
    static unsigned char palette_RGB[64][3] = {
	{0x00, 0x00, 0x00}, {0x00, 0x00, 0x15},
	{0x00, 0x00, 0x2A}, {0x00, 0x00, 0x3F},
	{0x00, 0x15, 0x00}, {0x00, 0x15, 0x15},
	{0x00, 0x15, 0x2A}, {0x00, 0x15, 0x3F},
	{0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x15},
	{0x00, 0x2A, 0x2A}, {0x00, 0x2A, 0x3F},
	{0x00, 0x3F, 0x00}, {0x00, 0x3F, 0x15},
	{0x00, 0x3F, 0x2A}, {0x00, 0x3F, 0x3F},
	{0x15, 0x00, 0x00}, {0x15, 0x00, 0x15},
	{0x15, 0x00, 0x2A}, {0x15, 0x00, 0x3F},
	{0x15, 0x15, 0x00}, {0x15, 0x15, 0x15},
	{0x15, 0x15, 0x2A}, {0x15, 0x15, 0x3F},
	{0x15, 0x2A, 0x00}, {0x15, 0x2A, 0x15},
	{0x15, 0x2A, 0x2A}, {0x15, 0x2A, 0x3F},
	{0x15, 0x3F, 0x00}, {0x15, 0x3F, 0x15},
	{0x15, 0x3F, 0x2A}, {0x15, 0x3F, 0x3F},
	{0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x15},
	{0x2A, 0x00, 0x2A}, {0x2A, 0x00, 0x3F},
	{0x2A, 0x15, 0x00}, {0x2A, 0x15, 0x15},
	{0x2A, 0x15, 0x2A}, {0x2A, 0x15, 0x3F},
	{0x2A, 0x2A, 0x00}, {0x2A, 0x2A, 0x15},
	{0x2A, 0x2A, 0x2A}, {0x2A, 0x2A, 0x3F},
	{0x2A, 0x3F, 0x00}, {0x2A, 0x3F, 0x15},
	{0x2A, 0x3F, 0x2A}, {0x2A, 0x3F, 0x3F},
	{0x3F, 0x00, 0x00}, {0x3F, 0x00, 0x15},
	{0x3F, 0x00, 0x2A}, {0x3F, 0x00, 0x3F},
	{0x3F, 0x15, 0x00}, {0x3F, 0x15, 0x15},
	{0x3F, 0x15, 0x2A}, {0x3F, 0x15, 0x3F},
	{0x3F, 0x2A, 0x00}, {0x3F, 0x2A, 0x15},
	{0x3F, 0x2A, 0x2A}, {0x3F, 0x2A, 0x3F},
	{0x3F, 0x3F, 0x00}, {0x3F, 0x3F, 0x15},
	{0x3F, 0x3F, 0x2A}, {0x3F, 0x3F, 0x3F}
    };

    /* Start writing at color 0. */
    outb (0x00, 0x03C8);

    /* Write all 64 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB, 64 * 3);
}

/*
 * fill_palette_mode_x_332
 *   DESCRIPTION: Fill VGA palette with necessary colors for the adventure
 *                game.  Only the first 64 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 64 palette colors
 */
void fill_palette_mode_x_332(){
	int i;
	uint8_t r, g, b;

    static unsigned char palette_RGB_8bit[256][3];
    for(i = 0; i < 256; i++){
    	r = ((uint8_t)i & 0xE0) >> 5;
    	g = ((uint8_t)i & 0x1C) >> 2;
    	b = (uint8_t)i & 0x3;
    	palette_RGB_8bit[i][0] = r << 3;
    	palette_RGB_8bit[i][1] = g << 3;
    	palette_RGB_8bit[i][2] = b << 4;
    }

    /* Start writing at color 0. */
    outb(0x00, 0x03C8);

    /* Write all 64 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB_8bit, 256 * 3);
}

/*
 * fill_palette_mode_x_custom
 *   DESCRIPTION: Fill VGA palette with necessary colors for the custom photo for
 *                game.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the palette colors for the rest of the colors
 */
void fill_palette_mode_x_custom(uint8_t * palette){
    /* Start writing at color 64. */
    outb (0x40, 0x03C8);

    /* Write all 64 colors from array. */
    REP_OUTSB (0x03C9, palette, 192 * 3);
}

/*
 * clear_screens
 *   DESCRIPTION: Fills the video memory with zeroes.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: fills all 256kB of VGA video memory with zeroes
 */
void clear_screens(){
    /* Write to all four planes at once. */
    SET_WRITE_MASK (0x0F00);

    /* Set 64kB to zero (times four planes = 256kB). */
    memset((void*)fb_addr, 0, MODE_X_MEM_SIZE);
    // memset((void*)VGA_VIDEO + 0x3E7F, 60, MODE_X_MEM_SIZE);
}

/*
 * draw_rectangle
 *   DESCRIPTION: 	screen_x	- position x to draw rect
					screen_y	- position y to draw rect
					color		- color to use in palette
					rect_x		- x dim of rect
					rect_y		- y dim of rect
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: draws rectangle on screen
 */
void draw_rectangle(int screen_x, int screen_y, uint8_t color, int rect_x, int rect_y){
	int i, j;

	// put the colors into the build buffer
	for(i = 0; i < rect_y; i++){
		for(j = 0; j < rect_x; j++){
			plot_pixel(screen_x + j, screen_y + i, color);
		}
	}
	// show_screen();
}

/*
 * draw_circle
 *   DESCRIPTION: 	x - x location of the center
 					y - y location of the center
 					r - radius of the circle
 					color - color of the circle
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots circle on screen
 */
void draw_circle(int x, int y, int r, int color){
	int i, j;

	for(i = 0; i < SCREEN_Y_DIM; i++){
		for(j = 0; j < SCREEN_X_DIM; j++){
			if((j - x) * (j - x) + (i - y) * (i - y) <= (r * r)){
				plot_pixel(j, i, color);
			}
		}
	}
	// show_screen();
}


/*
 * plot_pixel
 *   DESCRIPTION: Plots the pixel into build buffer in vga memory format
 *   INPUTS: 	x 		- screen x pos
 				y 		- screen y pos
 				color 	- color to plot
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void plot_pixel(int x, int y, uint8_t color){
	int idx = (y * SCREEN_X_DIM + x); 				// index of pixel row major form
	int p_off = idx & 3; 						// which plane we are in
	int p_idx = idx >> 2; 							// which index we are in within that plane
	// build_buf[p_off * PLANE_DIM + p_idx] = color; 	// plot pixel into buffer
	screen_buf[y * SCREEN_X_DIM + x] = color;
	SET_WRITE_MASK(1 << (p_off + 8));							// set the write mask
	memcpy((void*)(fb_addr + p_idx), (void*)&color, 1); 		// p_off * PLANE_DIM +
	// build_buf[p_off * PLANE_DIM + p_idx] = color; 	// plot pixel into buffer
}

/*
 * get_pixel
 *   DESCRIPTION: Plots the pixel into build buffer in vga memory format
 *   INPUTS: 	x 		- screen x pos
 				y 		- screen y pos
 *   OUTPUTS: color - color at the screen location
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
uint8_t get_pixel(int x, int y){
	return screen_buf[y * SCREEN_X_DIM + x];
	// int idx = (y * SCREEN_X_DIM + x); 				// index of pixel row major form
	// int p_off = idx & 3; 						// which plane we are in
	// int p_idx = idx >> 2; 							// which index we are in within that plane
	// p_idx = p_idx;
	// // build_buf[p_off * PLANE_DIM + p_idx] = color; 	// plot pixel into buffer

	// SET_WRITE_MASK(1 << (p_off + 8));							// set the write mask
	// return (((uint8_t*)fb_addr)[p_idx] & (0xFF << (p_off << 3)));
	// return 0;
	// build_buf[p_off * PLANE_DIM + p_idx] = color; 	// plot pixel into buffer
}

/*
 * show_screen
 *   DESCRIPTION: Show the logical view window on the video display.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies from the build buffer to video memory;
 *                 shifts the VGA display source to point to the new image
 */
void show_screen(){
    // int p_off;            /* plane offset of first display plane */
    // int i;		  /* loop index over video planes        */

	/* Draw to each plane in the video memory. */
	// for (i = 0; i < 4; i++) {
	// 	p_off = 3 - i;
	// 	SET_WRITE_MASK(1 << (p_off + 8));
	// 	memcpy((void*)(fb_addr), (void*)(build_buf + p_off * PLANE_DIM), PLANE_DIM);
	// }

	// SET_WRITE_MASK (0x0F00);
	// memcpy((void*)fb_addr, (void*)build_buf, SCREEN_DIM);

	/*
	 * Change the VGA registers to point the top left of the screen
	 * to the video memory that we just filled.
	 */
	outw (((fb_addr - VGA_VIDEO) & 0xFF00) | 0x0C, 0x03D4);
	outw ((((fb_addr - VGA_VIDEO) & 0x00FF) << 8) | 0x0D, 0x03D4);

	/* Switch to the other target screen in video memory. */
	fb_addr ^= 0x4000;
}

/*
 * draw_mouse_cursor
 *   DESCRIPTION: 	curr_x - pointer to cursor x
 					curr_y - pointer to cursor y
 					dx 	   - delta x
 					dy 	   - delta y
 					frames - number of frames to move
 					sx 	   - small movements in x
 					sy 	   - small movements in y
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: moves the cursor to next location
 */
void draw_mouse_cursor(int * curr_x, int * curr_y, int dx, int dy, int frames, int sx, int sy){
	int i;
	int save_x, save_y;
	int * prev_x;
	int * prev_y;
	uint8_t* save_buf;
	frames = frames == 0 ? 1 : frames;

	for(i = 0; i < frames; i++){
		save_buf = (fb_addr == VGA_VIDEO) ? cursor_save1 : cursor_save2;
		prev_x = (fb_addr == VGA_VIDEO) ? &fb1_mouse_x_prev : &fb2_mouse_x_prev;
		prev_y = (fb_addr == VGA_VIDEO) ? &fb1_mouse_y_prev : &fb2_mouse_y_prev;

		// write original screen
		for(save_y = 0; save_y < VGA_CURSOR_SIZE; save_y++){
			for(save_x = 0; save_x < VGA_CURSOR_SIZE; save_x++){
				// save_buf[save_y * VGA_CURSOR_SIZE + save_x] = get_pixel(*curr_x + save_x, *curr_y + save_y);
				plot_pixel(*prev_x + save_x, *prev_y + save_y, save_buf[save_y * VGA_CURSOR_SIZE + save_x]);
			}
		}

		*curr_x += dx / frames == 0 ? sx : dx / frames;
		*curr_y -= dy / frames == 0 ? sy : dy / frames;

		*curr_x = (*curr_x + VGA_CURSOR_SIZE) >= SCREEN_X_DIM ? SCREEN_X_DIM - VGA_CURSOR_SIZE : *curr_x;
		*curr_y = (*curr_y + VGA_CURSOR_SIZE) >= SCREEN_Y_DIM ? SCREEN_Y_DIM - VGA_CURSOR_SIZE : *curr_y;

		*curr_x = *curr_x < 0 ? 0 : *curr_x;
		*curr_y = *curr_y < 0 ? 0 : *curr_y;

		// save new screen
		for(save_y = 0; save_y < VGA_CURSOR_SIZE; save_y++){
			for(save_x = 0; save_x < VGA_CURSOR_SIZE; save_x++){
				save_buf[save_y * VGA_CURSOR_SIZE + save_x] = get_pixel(*curr_x + save_x, *curr_y + save_y);
			}
		}

		// draw_rectangle(*curr_x, *curr_y, 3, VGA_CURSOR_SIZE, VGA_CURSOR_SIZE);
		plot_cursor(*curr_x, *curr_y);

		// show_screen(); 				// whether to double buffer

		*prev_x = *curr_x;
		*prev_y = *curr_y;

		if(!(dx / frames) && !(dy / frames)){
			break;
		}
	}
	// draw_rectangle(*curr_x, *curr_y, 3, VGA_CURSOR_SIZE, VGA_CURSOR_SIZE);
}

/*
 * draw_image_322
 *   DESCRIPTION: 	img - pointer to array of pixels
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots image on screen
 */
void draw_image_322(uint8_t * img){
	int y, x;

	for(y = 0; y < SCREEN_Y_DIM; y++){
		for(x = 0; x < SCREEN_X_DIM; x++){
			plot_pixel(x, y, img[y * SCREEN_Y_DIM + (x + y * 120)]);
		}
	}
}

/*
 * draw_image_565
 *   DESCRIPTION: 	img - pointer to array of pixels
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots image on screen
 */
void draw_image_565(pixel_565_t * img){
	int y, x;

	fill_palette_mode_x_custom(get_palette(img));

	for(y = 0; y < SCREEN_Y_DIM; y++){
		for(x = 0; x < SCREEN_X_DIM; x++){
			plot_pixel(x, y, (uint8_t)(img[y * SCREEN_Y_DIM + (x + y * 120)].pixel & 0x00FF));
		}
	}
}

/*
 * draw_image_565_from_file
 *   DESCRIPTION: 	img - pointer to array of pixels
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: plots image on screen
 */
void draw_image_565_from_file(char * fname){
	dentry_t dentry;
	pixel_565_t img[SCREEN_X_DIM * SCREEN_Y_DIM];

	// get the dentry information
	read_dentry_by_name((uint8_t*)fname, &dentry);

	// read out the file into the buffer
	read_data(dentry.inode, 0, (uint8_t*)img, FILE_MAX_LEN);

	// draw the image
	draw_image_565(img);
}

/*
 * get_cursor_image
 *   DESCRIPTION: 	buf - pointer to array of pixels
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: loads cursor image into the buffer
 */
void get_cursor_image(pixel_565_t * buf){
	dentry_t dentry;

	// get the dentry information
	read_dentry_by_name((uint8_t*)"cursor.bin", &dentry);

	// read out the file into the buffer
	read_data(dentry.inode, 0, (uint8_t*)buf, FILE_MAX_LEN);
}

/*
 * plot_cursor
 *   DESCRIPTION: 	x - x location of pointer
 					y - y loaction of pointer
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: draws cursor to screen
 */
void plot_cursor(int x, int y){
	int i, j;
	for(i = 0; i < VGA_CURSOR_SIZE; i++){
		for(j = 0; j < VGA_CURSOR_SIZE; j++){
			switch(cursor_img[i * VGA_CURSOR_SIZE + j].pixel){
				case WHITE_PIX:
					plot_pixel(x + j, y + i, WHITE_PIX);
					break;

				case BLACK_PIX:
					plot_pixel(x + j, y + i, BLACK_COL);
					break;

				default:;
			}
		}
	}
}

/*
 * set_seq_regs_and_reset
 *   DESCRIPTION: Set VGA sequencer registers and miscellaneous output
 *                register; array of registers should force a reset of
 *                the VGA sequencer, which is restored to normal operation
 *                after a brief delay.
 *   INPUTS: table -- table of sequencer register values to use
 *           val -- value to which miscellaneous output register should be set
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS], unsigned char val){
    /*
     * Dump table of values to sequencer registers.  Includes forced reset
     * as well as video blanking.
     */
    REP_OUTSW (0x03C4, table, NUM_SEQUENCER_REGS);

    /* Delay a bit... */
    {volatile int ii; for (ii = 0; ii < 10000; ii++);}

    /* Set VGA miscellaneous output register. */
    outb (val, 0x03C2);

    /* Turn sequencer on (array values above should always force reset). */
    outw (0x0300, 0x03C4);
}

/*
 * clear_mode_X
 *   DESCRIPTION: Puts the VGA into text mode 3 (color text).
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: restores font data to video memory; clears screens;
 *                 unmaps video memory; checks memory fence integrity
 */
void clear_mode_X(){
    int i;   /* loop index for checking memory fence */

    /* Put VGA into text mode, restore font data, and clear screens. */
    set_text_mode_3 (1);

    /* Unmap video memory. */
    //(void)munmap (mem_image, VID_MEM_SIZE);

    /* Check validity of build buffer memory fence.  Report breakage. */
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
	if (build[i] != MEM_FENCE_MAGIC) {
	    puts ("lower build fence was broken");
	    break;
	}
    }
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
        if (build[BUILD_BUF_SIZE + MEM_FENCE_WIDTH + i] != MEM_FENCE_MAGIC) {
	    puts ("upper build fence was broken");
	    break;
	}
    }
}

/*
 * write_font_data
 *   DESCRIPTION: Copy font data into VGA memory, changing and restoring
 *                VGA register values in order to do so.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: leaves VGA registers in final text mode state
 */
void write_font_data(){
    int i;                /* loop index over characters                   */
    int j;                /* loop index over font bytes within characters */
    unsigned char* fonts; /* pointer into video memory                    */

    /* Prepare VGA to write font data into video memory. */
    outw (0x0402, 0x3C4);
    outw (0x0704, 0x3C4);
    outw (0x0005, 0x3CE);
    outw (0x0406, 0x3CE);
    outw (0x0204, 0x3CE);

    /* Copy font data from array into video memory. */
    for (i = 0, fonts = mem_image; i < 256; i++) {
	       for (j = 0; j < 16; j++)
	          fonts[j] = font_data[i][j];
	       fonts += 32; /* skip 16 bytes between characters */
    }

    /* Prepare VGA for text mode. */
    outw (0x0302, 0x3C4);
    outw (0x0304, 0x3C4);
    outw (0x1005, 0x3CE);
    outw (0x0E06, 0x3CE);
    outw (0x0004, 0x3CE);
}

/*
 * set_text_mode_3
 *   DESCRIPTION: Put VGA into text mode 3 (color text).
 *   INPUTS: clear_scr -- if non-zero, clear screens; otherwise, do not
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: may clear screens; writes font data to video memory
 */
void set_text_mode_3(int clear_scr){
    unsigned long* txt_scr; /* pointer to text screens in video memory */
    int i;                  /* loop over text screen words             */

    //VGA_blank (1);                               /* blank the screen        */
    /*
     * The value here had been changed to 0x63, but seems to work
     * fine in QEMU (and VirtualPC, where I got it) with the 0x04
     * bit set (VGA_MIS_DCLK_28322_720).
     */
    set_seq_regs_and_reset (text_seq, 0x67);     /* sequencer registers     */
    set_CRTC_registers (text_CRTC);              /* CRT control registers   */
    set_attr_registers (text_attr);              /* attribute registers     */
    set_graphics_registers (text_graphics);      /* graphics registers      */
    fill_palette_text();			     		/* palette colors          */
    if (clear_scr) {				            /* clear screens if needed */
		txt_scr = (unsigned long*)(mem_image + 0x18000);
		for (i = 0; i < 8192; i++)
		    *txt_scr++ = 0x07200720;
    }
	// clear();
    write_font_data ();                          /* copy fonts to video mem */
    //VGA_blank (0);			                      /* unblank the screen      */
}
