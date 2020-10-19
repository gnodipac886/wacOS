#ifndef _PAGING_H
#define _PAGING_H

#define ALIGN_4KB 			12
#define SHFT_4MB_ADDR 		10

#define NUM_PAGE_DIR 		1024
#define NUM_PAGE_TB 		1024

#define VIDEO_MEM_IDX		0xB8

extern void __init_paging__();

#endif /* _PAGING_H */
