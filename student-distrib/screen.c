#include "lib.h"
#include "screen.h"

/* 
 * macro used to write an array of two-byte values to two consecutive ports 
 */
#define REP_OUTSW(port,source,count)                                    \
do {                                                                    \
    asm volatile ("                                                     \
     1: movw 0(%1),%%ax                                                ;\
	outw %%ax,(%w2)                                                ;\
	addl $2,%1                                                     ;\
	decl %0                                                        ;\
	jne 1b                                                          \
    " : /* no outputs */                                                \
      : "c" ((count)), "S" ((source)), "d" ((port))                     \
      : "eax", "memory", "cc");                                         \
} while (0)

/* 
 * macro used to write an array of one-byte values to two consecutive ports 
 */
#define REP_OUTSB(port,source,count)                                    \
do {                                                                    \
    asm volatile ("                                                     \
     1: movb 0(%1),%%al                                                ;\
	outb %%al,(%w2)                                                ;\
	incl %1                                                        ;\
	decl %0                                                        ;\
	jne 1b                                                          \
    " : /* no outputs */                                                \
      : "c" ((count)), "S" ((source)), "d" ((port))                     \
      : "eax", "memory", "cc");                                         \
} while (0)

/* 
 * macro used to target a specific video plane or planes when writing
 * to video memory in mode X; bits 8-11 in the mask_hi_bits enable writes
 * to planes 0-3, respectively
 */
#define SET_WRITE_MASK(mask_hi_bits)                                    \
do {                                                                    \
    asm volatile ("                                                     \
	movw $0x03C4,%%dx    	/* set write mask                    */;\
	movb $0x02,%b0                                                 ;\
	outw %w0,(%%dx)                                                 \
    " : : "a" ((mask_hi_bits)) : "edx", "memory");                      \
} while (0)

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
	fill_palette_mode_x ();			 				/* palette colors        */
	clear_screens();								/* set video memory      */
	VGA_blank (0);			         				/* unblank the screen    */
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
 * fill_palette_mode_x
 *   DESCRIPTION: Fill VGA palette with necessary colors for the adventure 
 *                game.  Only the first 64 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 64 palette colors
 */   
void fill_palette_mode_x(){
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
    outb(0x00, 0x03C8);

    /* Write all 64 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB, 64 * 3);
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
    memset((void*)VGA_VIDEO, 1, MODE_X_MEM_SIZE);
}
