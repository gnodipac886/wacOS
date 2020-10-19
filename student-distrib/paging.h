#ifndef _PAGING_H
#define _PAGING_H

#define ALIGN_4KB 			12   // alignment for 4kb paging
#define SHFT_4MB_ADDR 		10 // Memory address interval

#define NUM_PAGE_DIR 		1024  // total space for page directory
#define NUM_PAGE_TB 		1024  // total space for page table

#define VIDEO_MEM_IDX		0xB8  // video memory index

/* initializing paging */
extern void __init_paging__();

#endif /* _PAGING_H */
