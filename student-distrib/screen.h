#ifndef _SCREEN_H
#define _SCREEN_H

#define NUM_GRAPHICS_REGS       9
#define NUM_CRTC_REGS          	25
#define NUM_ATTR_REGS			22
#define MODE_X_MEM_SIZE     	65536
#define VIDEO       			0xB8000
#define VGA_VIDEO       		0xA0000

void __screen_init__();
void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]);
void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS]);
void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]);
void VGA_blank (int blank_bit);
void fill_palette_mode_x();
void clear_screens();


#endif /* _SCREEN_H */
