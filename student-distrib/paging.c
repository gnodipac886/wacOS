#include "paging.h"
#include "x86_desc.h"
#include "types.h"

/* __init_paging__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initializing paging, sets up page directory, page table
 *		Side Effects: none
 */
void __init_paging__(){
	// counter for for loops
	int i;

	// set the first page directory to have 4kb set up
	page_directory[0].addr 				= 	(uint32_t)(page_table) >> ALIGN_4KB;			// address to the page_table
	page_directory[0].accessed 			= 	0;												// not used, set to 0
	page_directory[0].global 			= 	0;												// we only set the page for kernel page
	page_directory[0].size 				= 	0;												// 0 for 4kB entry
	page_directory[0].available 		= 	0;												// not used, set to 0
	page_directory[0].dirty 			= 	0; 												// for 4kB PDE, we set it to 0
	page_directory[0].cache_disable 	= 	0; 												// contains video memory so 0
	page_directory[0].write_through		= 	0; 												// we always want write back
	page_directory[0].user_supervisor 	= 	0; 												// contains video memory, should be supervisor
	page_directory[0].read_write 		= 	1; 												// all pages are read write
	page_directory[0].present 			= 	1; 												// all valid PDE needs to be set to 1

	// set the second page directory for kernel usage
	page_directory[1].addr 				= 	1 << SHFT_4MB_ADDR;								// address to the page_table
	page_directory[1].accessed 			= 	0;												// not used, set to 0
	page_directory[1].dirty 			= 	0;												// not used, set to 0
	page_directory[1].global 			= 	1;												// we only set the page for kernel page
	page_directory[1].size 				= 	1;												// 1 for 4MB entry
	page_directory[1].available 		= 	0;												// not used, set to 0
	page_directory[1].cache_disable 	= 	1; 												// set to 1 since kernel code
	page_directory[1].write_through		= 	0; 												// we always want write back
	page_directory[1].user_supervisor 	= 	0; 												// kernel code should be supervisor
	page_directory[1].read_write 		= 	1; 												// all pages are read write
	page_directory[1].present 			= 	1; 												// all valid PDE needs to be set to 1

	// example for virtual maping
	// page_directory[32].addr 				= 	3 << SHFT_4MB_ADDR;								// address to the page_table
	// page_directory[32].accessed 			= 	0;												// not used, set to 0
	// page_directory[32].dirty 			= 	0;												// not used, set to 0
	// page_directory[1].global 			= 	1;												// we only set the page for kernel page
	// page_directory[1].size 				= 	1;												// 1 for 4MB entry
	// page_directory[1].available 		= 	0;												// not used, set to 0
	// page_directory[1].cache_disable 	= 	1; 												// set to 1 since kernel code
	// page_directory[1].write_through		= 	0; 												// we always want write back
	// page_directory[1].user_supervisor 	= 	0; 												// kernel code should be supervisor
	// page_directory[1].read_write 		= 	1; 												// all pages are read write
	// page_directory[1].present 			= 	1; 												// all valid PDE needs to be set to 1

	// set the rest of the directory for other memory spaces
	for(i = 2; i < NUM_PAGE_DIR; i++){
		page_directory[i].addr 				= 	i << SHFT_4MB_ADDR;							// address to the page_table
		page_directory[i].accessed 			= 	0;											// not used, set to 0
		page_directory[i].dirty 			= 	0;											// not used, set to 0
		page_directory[i].global 			= 	0;											// we only set the page for kernel page
		page_directory[i].size 				= 	1;											// 1 for 4MB entry
		page_directory[i].available 		= 	0;											// not used, set to 0
		page_directory[i].cache_disable 	= 	1; 											// we can cache since no volatile memory
		page_directory[i].write_through		= 	0; 											// we always want write back
		page_directory[i].user_supervisor 	= 	1; 											// user level memory
		page_directory[i].read_write 		= 	1; 											// all pages are read write
		page_directory[i].present 			= 	0; 											// all valid PDE needs to be set to 1
	}

	// set up the page table
	for(i = 0; i < NUM_PAGE_TB; i++){
		page_table[i].addr 				= 	i;												// address to the page_table
		page_table[i].accessed 			= 	0;												// not used, set to 0
		page_table[i].dirty 			= 	0;												// not used, set to 0
		page_table[i].global 			= 	0;												// we only set the page for kernel page
		page_table[i].size 				= 	0;												// 0 for page attribute table
		page_table[i].available 		= 	0;												// not used, set to 0
		page_table[i].cache_disable 	= 	i == VIDEO_MEM_IDX ? 0 : 1;   					// volatile for video mem mapped IO set to 0, otherwise 1
		page_table[i].write_through		= 	0; 												// we always want write back
		page_table[i].user_supervisor 	= 	1; 												// user level memory
		page_table[i].read_write 		= 	1; 												// all pages are read write
		page_table[i].present 			= 	i != VIDEO_MEM_IDX ? 0 : 1; 					// all valid PDE needs to be set to 1
	}

	// set up CRX registers
	asm(
		"movl 	%0, 			%%eax;"		// move page directory into eax
		"movl 	%%eax, 			%%cr3;"		// move page directory address into cr3

		"movl 	%%cr4, 			%%eax;"		// dump out cr4
		"orl 	$0x00000010, 	%%eax;"		// or the 4th bit of cr4
		"movl 	%%eax, 			%%cr4;"		// put eax contents back into cr4

		"movl 	%%cr0, 			%%eax;"		// dump out cr0
		"orl 	$0x80000000,	%%eax;"		// make the first and last bits 1
		"movl 	%%eax, 			%%cr0;"		// put it back into cr0

		:							// not outputs yet
		:"r"(page_directory) 		// input is page directory
		:"%eax" 					// clobbered register

		);
}
