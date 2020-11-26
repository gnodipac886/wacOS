#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define ALIGN_4KB 			    12      // alignment for 4kb paging
#define SHFT_4MB_ADDR 		    10      // Memory address interval

#define NUM_PAGE_DIR 		    1024    // total space for page directory
#define NUM_PAGE_TB 		    1024    // total space for page table

#define VIDEO_MEM_IDX		    0xB8    // video memory index
#define BACKGROUND_BUF1_IDX     0xB9    // terminal 1's background buffer index in page table
#define BACKGROUND_BUF2_IDX     0xBA    // terminal 2's background buffer index in page table
#define BACKGROUND_BUF3_IDX     0xBB    // terminal 3's background buffer index in page table

#define USER_PAGE               128 >> 2// user page memory location (page directory index)
#define VIDMAP_4MB_PAGE         33      // vidmap page memory location (page directory index)

/* initializing paging */
extern void __init_paging__();

/* paging for execute */
extern int exe_paging(int pid, int present);

/* paging for vidmap */
extern int vidmap_pte_setup(uint8_t** screen_start, uint8_t present);

/* 4kB text-screen paging for scheduler */
void text_screen_map_update(int curr_scheduled, int curr_screen);

/* switch vidmap to the right video memory locaiton */
void vidmap_update();

/* flushes the tlb */
void flush_tlb();

#endif /* _PAGING_H */
