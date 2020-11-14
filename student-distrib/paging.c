#include "paging.h"
#include "x86_desc.h"

/* __init_paging__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initializing paging, sets up page directory, page table
 *		Side Effects: none
 */
void __init_paging__(){
	// counter for for loops
	int i;

	// set the first page directory entry to have 4kb set up
	page_directory[0].addr 				= 	(uint32_t)(page_table) >> ALIGN_4KB;			// address to the page_table
	page_directory[0].accessed 			= 	0;												// not used, set to 0
	page_directory[0].global 			= 	0;												// we only set the page for kernel page
	page_directory[0].size 				= 	0;												// 0 for 4kB entry
	page_directory[0].available 		= 	0;												// not used, set to 0
	page_directory[0].dirty 			= 	0; 												// for 4kB PDE, we set it to 0
	page_directory[0].cache_disable 	= 	0; 												// contains video memory so 0
	page_directory[0].write_through		= 	0; 												// we always want write back
	page_directory[0].user_supervisor 	= 	0; 												// contains video memory
	page_directory[0].read_write 		= 	1; 												// all pages are read write
	page_directory[0].present 			= 	1; 												// all valid PDE needs to be set to 1

	// set the second page directory entry for kernel usage
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

/* exe_paging
 *      Inputs: pid 	- process id inside of PCB
 				present - whether the present bit should be on or off
 *      Return Value: 0 -- paging set up correclty
 *					  -1 -- pid value invalid
 *      Function: 128MB in virtual memory (user page) will map to physical memory for the tasks starting at 8MB
 *      Side Effects: Flushes the TLB after mapping
 */
int exe_paging(int pid, int present){
	// check for a valid process id
	if (pid < 0){
		return -1;
	}
	// mapping user page to physical memory for tasks
	page_directory[USER_PAGE].addr 				= 	(2 + pid)  << SHFT_4MB_ADDR;					// address to the tasks starting at 8MB, 2 to skip first two entries
	page_directory[USER_PAGE].accessed 			= 	0;												// not used, set to 0
	page_directory[USER_PAGE].dirty 			= 	0;												// not used, set to 0
	page_directory[USER_PAGE].global 			= 	1;												// we only set the page for kernel page
	page_directory[USER_PAGE].size 				= 	1;												// 1 for 4MB entry
	page_directory[USER_PAGE].available 		= 	0;												// not used, set to 0
	page_directory[USER_PAGE].cache_disable 	= 	1; 												// set to 1 for program code
	page_directory[USER_PAGE].write_through		= 	0; 												// we always want write back
	page_directory[USER_PAGE].user_supervisor 	= 	1; 												// user-level
	page_directory[USER_PAGE].read_write 		= 	1; 												// all pages are read write
	page_directory[USER_PAGE].present 			= 	present; 										// page available

	// flush the TLB
	asm(
		"movl 	%0, 			%%eax;"		// move page directory into eax
		"movl 	%%eax, 			%%cr3;"		// move page directory address into cr3

		:							// not outputs yet
		:"r"(page_directory) 		// input is page directory
		:"%eax" 					// clobbered register

		);

	return 0;
}

/*  vidmap_pte_setup
 * 		Inputs: screen_start - double ptr, an addr in virtual mem user page
 				present 	 - whether we want allocate or deallocate
 * 		Return Value: 0 -- paging is setup
 * 					  -1 -- failure/screen_start val is invalid
 * 		Function: Sets up paging for vidmap; stores virtual addr of new 4kB videomem page into where screen_start points to. 
 * 		Side Effects: Flushes TLB after mapping
 *  
 */

int vidmap_pte_setup(uint8_t ** screen_start, uint8_t present) {
	if (screen_start == NULL && present) {																	// check if screen_start argument is valid
		return -1;
	}

	// initialize page table if we haven't already
		// set the vidmap page directory entry to have 4kb set up
	page_directory[VIDMAP_4MB_PAGE].addr 				= 	(uint32_t)(vidmap_page_table) >> ALIGN_4KB;		// address to the page_table
	page_directory[VIDMAP_4MB_PAGE].accessed 			= 	0;												// not used, set to 0
	page_directory[VIDMAP_4MB_PAGE].global 				= 	0;												// ignored for 4K page directory entries
	page_directory[VIDMAP_4MB_PAGE].size 				= 	0;												// 0 for 4kB entry
	page_directory[VIDMAP_4MB_PAGE].available 			= 	0;												// not used, set to 0
	page_directory[VIDMAP_4MB_PAGE].dirty 				= 	0; 												// for 4kB PDE, we set it to 0
	page_directory[VIDMAP_4MB_PAGE].cache_disable 		= 	0; 												// contains video memory so 0
	page_directory[VIDMAP_4MB_PAGE].write_through		= 	0; 												// we always want write back
	page_directory[VIDMAP_4MB_PAGE].user_supervisor 	= 	1; 												// contains video memory, should be supervisor
	page_directory[VIDMAP_4MB_PAGE].read_write 			= 	1; 												// all pages are read write
	page_directory[VIDMAP_4MB_PAGE].present 			= 	present; 										// all valid PDE needs to be set to 1

	// set up the vidmap page table
	vidmap_page_table[0].addr 							= 	VIDEO_MEM_IDX;									// address to the page_table
	vidmap_page_table[0].accessed 						= 	0;												// not used, set to 0
	vidmap_page_table[0].dirty 							= 	0;												// not used, set to 0
	vidmap_page_table[0].global 						= 	0;												// we only set the page for kernel page
	vidmap_page_table[0].size 							= 	0;												// 0 for page attribute table
	vidmap_page_table[0].available 						= 	0;												// not used, set to 0
	vidmap_page_table[0].cache_disable 					= 	0;   											// volatile for video mem mapped IO set to 0, otherwise 1
	vidmap_page_table[0].write_through					= 	0; 												// we always want write back
	vidmap_page_table[0].user_supervisor 				= 	1; 												// user level memory
	vidmap_page_table[0].read_write 					= 	1; 												// all pages are read write
	vidmap_page_table[0].present 						= 	present; 										// all valid PDE needs to be set to 1

	// flush the TLB
	asm(
		"movl 	%0, 			%%eax;"																		// move page directory into eax
		"movl 	%%eax, 			%%cr3;"																		// move page directory address into cr3

		:																									// not outputs yet
		:"r"(page_directory) 																				// input is page directory
		:"%eax" 																							// clobbered register
	);
	//}

	if (screen_start != NULL && present) {																	// check if screen_start argument is valid
		*screen_start = (uint8_t*)(VIDMAP_4MB_PAGE<<22);													// page 33 shift up 22 times to align to 4MB
	}
	return 0;

}
