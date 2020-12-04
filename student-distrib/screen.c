#include "lib.h"
#include "screen.h"
#include "paging.h"
#include "octree.h"
#include "filesystem.h"
// #include "../images/big_sur.h"
// #include "../images/bar.h"

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

uint32_t fb_addr = (uint32_t)VGA_VIDEO; 			// address of current frame buffer we are using (0 based, 0xA0000 = 0x0)
uint8_t build_buf[320 * 200];

uint8_t cursor_save1[VGA_CURSOR_SIZE * VGA_CURSOR_SIZE];
uint8_t cursor_save2[VGA_CURSOR_SIZE * VGA_CURSOR_SIZE];

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
	VGA_blank (1);                               	/* blank the screen      */
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
	while(1)
	draw_image_565_from_file("big_sur.bin");
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
	int idx = (y * SCREEN_X_DIM + x); 				// index of pixel row major form
	int p_off = idx & 3; 						// which plane we are in
	int p_idx = idx >> 2; 							// which index we are in within that plane
	p_idx = p_idx;
	// build_buf[p_off * PLANE_DIM + p_idx] = color; 	// plot pixel into buffer

	SET_WRITE_MASK(1 << (p_off + 8));							// set the write mask
	// return (fb_addr)[p_idx] & (0xFF << (p_off << 3));
	return 0;
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

		// draw_rectangle(*prev_x, *prev_y, 0, VGA_CURSOR_SIZE, VGA_CURSOR_SIZE);

		// write original screen
		for(save_y = 0; save_y < VGA_CURSOR_SIZE; save_y++){
			for(save_x = 0; save_x < VGA_CURSOR_SIZE; save_x++){
				// save_buf[save_y * VGA_CURSOR_SIZE + save_x] = get_pixel(curr_x + save_x, curr_y + save_y);
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

		draw_rectangle(*curr_x, *curr_y, 3, VGA_CURSOR_SIZE, VGA_CURSOR_SIZE);

		// show_screen(); 				// whether to double buffer

		*prev_x = *curr_x;
		*prev_y = *curr_y;

		if(!(dx / frames) && !(dy / frames)){
			break;
		}
	}
	draw_rectangle(*curr_x, *curr_y, 3, VGA_CURSOR_SIZE, VGA_CURSOR_SIZE);
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
