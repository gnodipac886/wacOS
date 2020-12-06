#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define ALIGN_4KB 			12     // alignment for 4kb paging
#define SHFT_4MB_ADDR 		10     // Memory address interval

#define NUM_PAGE_DIR 		1024   // total space for page directory
#define NUM_PAGE_TB 		1024   // total space for page table

#define VIDEO_MEM_IDX		0xB8   // video memory index
#define VGA_IDX_START       0xA0   // VGA video memory index
#define VGA_IDX_END 		0xBF   // VGA video memory index
#define SB_IDX_START        0xC0   // sound blaster video memory start index
#define SB_IDX_END          0xFF   // sound blaster video memory end index

#define WAV_PAGE            10
#define USER_PAGE           32 	   // user page memory location (page directory index)
#define VIDMAP_4MB_PAGE     33     // vidmap page memory location (page directory index)

#define GUI_ACTIVATE 		0

/* initializing paging */
extern void __init_paging__();

/* paging for execute */
extern int exe_paging(int pid, int present);

/* paging for vidmap */
extern int vidmap_pte_setup(uint8_t** screen_start, uint8_t present);

void vga_mem_setup();
void sb_mem_setup();

#endif /* _PAGING_H */
