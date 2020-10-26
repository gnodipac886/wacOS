#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesystem.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 9; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}


/* divide_error_test
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Tests for the divide by 0 error interrupt
 *		Side Effects: none
 */
int divide_error_test(){
	TEST_HEADER;

	int i;
	int y;

	i = 0;
	y = 2 / i;						// divide random 2 by 0
	return 0;
}

/* invalid_opcode_test
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Tests for invalid opcode interrupt
 *		Side Effects: none
 */
int invalid_opcode_test() {
	TEST_HEADER;

	asm volatile(
		"ud2;"									// run the invalid opcode
		:
		:
		: "eax"
		);
	return 0;
}

/* overflow_test
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Test for overflow interrupt
 *		Side Effects: none
 */
int overflow_test() {
	TEST_HEADER;

	asm volatile(
		"movb 	$127, 	%%al;"				// try to add 127 in 8 bit al register to get OF flag
		"addb 	$127, 	%%al;"				// try to add 127 in 8 bit al register to get OF flag
		"into;"												// test for OF flag
		:
		:
		: "eax"
		);
	return 0;
}

/* bound_range_test
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Test for bound out of range interrupt using bound opcode
 *		Side Effects: none
 */
int bound_range_test() {
	// random
	int arr[2] = {1, 2};						// random array
	// checking for out of bound
	asm(
		"movl %0, %%eax;"	// move upper bound address of the array into eax
		"movl $3, %%ebx;"			// move out of bound index into ebx

		"bound %%ebx, (%%eax)"		// checking for bounds

		:							// no outputs
		:"r"(&arr) 		// input is array
		:"%eax", "%ebx"				// clobbered register
		);

	return 0;
}

/* Video Memory Paging Test
 *  Description: Check for proper video memory paging
 * 	Inputs: None
 *  Outputs: PASS - if no page fault
 *  Side Effects: None
 *  Function: dereferences first 10 video memory linear addresses
 */
int vid_mem_paging_test() {
	TEST_HEADER;

	int i, VM_data;
	int * VM_addr;

	for (i = 0; i < 9; i++){
		VM_addr = (int *)(0x000B8000 + i);					// 0x000B8000 for video memory location
		VM_data = *VM_addr;													// dereference the pointer
	}

	return PASS;

}


/* Kernel Paging Test
 *  Description: Check for proper Kernel paging
 * 	Inputs: None
 *  Outputs: PASS - if no page fault
 *  Side Effects: None
 *  Function: dereferences first 10 kernel linear addresses
 */
int kernel_paging_test() {
	TEST_HEADER;

	int i, kernel_data;
	int * kernel_addr;

	for (i = 0; i < 9; i++){									// loop through 9 different memory locations to be sure
		kernel_addr = (int *)(0x00400000 + i);	// 0x00400000 for kernel memory location
		kernel_data = *kernel_addr;							// dereference it
	}

	return PASS;

}

/* Unused Page Page Fault Test
 *  Description: Check for proper paging via addr in unused page
 * 	Inputs: None
 *  Outputs: PASS - if page fault
 *  Side Effects: None
 *  Function: dereferences addr in unused page
 */
int unused_paging_test() {
	TEST_HEADER;

	int * ptr;			//not in Video Memory nor Kernel page range
	int x;
	ptr = (int *)(0x080000B); // saving random pointer
	x = *ptr;				// dereferences the random pointer

	return 1;

}

//paging structure test
// info mem

/* system_call_test
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Test for system call functionality
 *		Side Effects: none
 */
/*

int system_call_test() {
	asm volatile(
		"movl $0x80, %%eax;"
		"call (%%eax);"
		: 								//no output operands yet
		:								//no input operands yet
		: "memory", "%eax", "%eip"		//clobbered registers
	);

	or something like.... asm volatile("int $0x80");

	return 0;
}
*/

/* deref_NULL_ptr_test
 * 		Inputs: none
 * 		Return Value: 1
 * 		Function: Tests paging, by deferencing a null pointer
 *		Side Effects: none
 */
int deref_NULL_ptr_test(){
	TEST_HEADER;
	int * ptr;
	int x;
	ptr = NULL; 		// saving NULL into pointer
	x = *ptr;				// trying to deference the NULL pointer

	return 1;
}


/* Checkpoint 2 tests */

/* read_dir
 *      Inputs: none
 *      Return Value: pass or fail
 *      Function: attempts to read out all the files in the filesystem
 *      Side Effects: none
 */
int read_dir(){
	// print out the header
	TEST_HEADER;

	// declare variables
	int fd;
	char curr_name[MAX_NAME_LEN + 1];

	// clear the scren
	clear();

	// attempts to open the directory
	fd = _open((uint8_t*)".");

	// see if the fd is valid
	if(fd == -1){
		return FAIL;
	}

	// keep read files until we hit the end and print out the name
	while(dir_read(fd, (void*)curr_name, 1024)){	// 1024 is random number
		// print out the name of the file
		printf("%s\n", curr_name);
	}

	// close file when done
	_close(fd);

	// return success if we pass
	return PASS;
}

/* read_dir
 *      Inputs: fname 		- the name fo the file we want to read
 *      Return Value: pass or fail
 *      Function: attempts to read out a whole file
 *      Side Effects: none
 */
int read_file(char * fname){
	// print test header
	TEST_HEADER;

	// declare variables
	int i = 0;
	int fd;
	int len = 100000;			// create a buffer length that is big enough
	char contents[len];

	// clear screen
	clear();

	// open the file
	fd = _open((uint8_t*)fname);

	// see if the fd is valid
	if(fd == -1){
		return FAIL;
	}

	// see if the file read of the contents is valid
	if(file_read(fd, contents, len) == -1){
		return FAIL;
	}

	// change the length to the length of the file
	len = _get_file_length(fd);

	// print out all the contents of the file
	while(i < len){
		// if we hit a null character, we skip it
		if(contents[i] == '\0'){
			i++;
			continue;
		}

		// otherwise, print the current character
		putc(contents[i]);

		// increment the counter
		i++;
	}

	// close the file when done
	_close(fd);

	return PASS;
}

/* read_long_name_file
 *      Inputs: none
 *      Return Value: pass or fail
 *      Function: checks if a long name file will pass or not when attempting to read
 *      Side Effects: none
 */
int read_long_name_file(){
	TEST_HEADER;													// print headder

	int fd;															// init fd

	clear(); 														// clear the screen
	fd = _open((uint8_t*)"verylargetextwithverylongname.txt");		// attempts to read the file

	if(fd == -1){													// if fails to read then function performs right
		return PASS;
	}

	_close(fd);	 													// otherwise function failed close the file

	return FAIL;
}

/* filesystem_sanity_check
 *      Inputs: none
 *      Return Value: pass or fail
 *      Function: performs a series of sanity checks on various filesystem functions
 *      Side Effects: none
 */
int filesystem_sanity_check(){
	TEST_HEADER;													// print headder

	// init variabes for function
	int test_results[6];											// holds all the test results from checks, 6 for num of tests
	int failed_flag = PASS;
	int i;
	char fake_name = 'a';

	clear();														// clears the screen

	test_results[0] = _open((uint8_t*)"thisfiledoesntexist.txt"); 	// open a file that doesn't exist

	test_results[1] = _open((uint8_t*)(&fake_name)); 				// open a file without a null terminating character

	test_results[2] = _close(8); 									// close out of bounds

	test_results[3] = _close(2); 									// close file that hasn't been opened at all

	test_results[4] = file_read(2, NULL, 0); 						// read a fd that's not been opened at all

	test_results[5] = _open(NULL); 									// test NULL pointer

	// loops through the test results and see which ones failed
	for(i = 0; i < 6; i ++){										// 6 for number of tests
		if(test_results[i] != -1){
			switch(i){
				case 0:
					printf("FAILED: file that doesn't exist\n");
					break;

				case 1:
					printf("FAILED: open file without null end\n");
					break;

				case 2:
					printf("FAILED: out of bounds fd\n");
					break;

				case 3:
					printf("FAILED: not open fd\n");
					break;

				case 4:
					printf("FAILED: read fd that's not open\n");
					break;

				case 5:
					printf("FAILED: NULL pointer test\n");
					break;

				default:
					break;
			}
			failed_flag = FAIL;										// set the flag to show that we failed
		}
	}
	return failed_flag;
}

/* test_rtc_freq
 *      Inputs: none
 *      Return Value: pass or fail
 *      Function: sets the rtc to different values and prints values at that rate
 *      Side Effects: none
 */
int test_rtc_freq(){
	TEST_HEADER;													// print headder

	// set up variables for function
	int i, ticks;
	int fd, buf;

	clear();														// clear the screen

	fd = _open((uint8_t*)"rtc");									// attempts to open the rtc file

	// check if the function is valid
	if(fd == -1){
		return FAIL;
	}

	// loop through possible frequencies and print values
	for(i = 2; i <= 1024; i++){										// 1024 that's the max rtc freq, 2 for lowest freq
		buf = i;
		if(_write(fd, (void*)(&buf), 100) != -1){ 					// attempts to set the rtc
			for(ticks = 0; ticks < (i * 2); ticks++){				// set up ticks for how long we want to print, 2 just random value for time
				rtc_read(fd, (void*)(&buf), 1000);					// wait for rtc to interrupt and 1000 is random
				printf("%d", i);									// we print the current frequency value
			}
		}
	}

	_close(fd);														// close the rtc file

	clear();														// clear the screen since its all clogged up
	return PASS;
}

/* vert_scroll_test
 * 		Inputs: none
 * 		Return Value: pass or fail
 * 		Function: Test for vertical scroll by printing over 25 lines
 *		Side Effects: none
 */
int vert_scroll_test(){
	int i; 			// counter

	// print lines to see scrolling work
	for(i = 0; i < 392; i++){										// 392 random
		printf("Testing Vertical Scroll: Line %d \n", i);
	}

	return PASS;
}

/* term_read_write_test
 * 		Inputs: none
 * 		Return Value: pass or fail
 * 		Function: read form the terminal buffer and echos it back out
 *		Side Effects: none
 */
int term_read_write_test(){
	// 128 in here for 128 characters possible in the buffer
	char buf[128];													// set up the buffer for the typing
	 while(1){														// keep echoing
		terminal_read(0, buf, 128);									// read from the keyboard
		terminal_write(1, buf, 128);								// echo the typings
 	}
	return PASS;
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/*CHECKPOINT 1*/
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("divide by 0 test", divide_error_test());
	// TEST_OUTPUT("invalid_opcode_test", invalid_opcode_test());
	// TEST_OUTPUT("overflow_test", overflow_test());
	// TEST_OUTPUT("bound range test", bound_range_test());
	// TEST_OUTPUT("video memory paging test", vid_mem_paging_test());
	// TEST_OUTPUT("kernel paging test", kernel_paging_test());
	// TEST_OUTPUT("unused page page fault test", unused_paging_test());
	// TEST_OUTPUT("deref_NULL_ptr_test", deref_NULL_ptr_test());

	/*CHECKPOINT 2*/
	// TEST_OUTPUT("read_dir", read_dir());
	// TEST_OUTPUT("reading a file", read_file("verylargetextwithverylongname.tx"));
	// TEST_OUTPUT("reading file with name too long", read_long_name_file());
	// TEST_OUTPUT("Sanity checks for filesystem", filesystem_sanity_check());
	// TEST_OUTPUT("Testing RTC", test_rtc_freq());
	// TEST_OUTPUT("vertical scroll test", vert_scroll_test());
	// TEST_OUTPUT("terminal read/write from keyboard test", term_read_write_test());

	// launch your tests here
}
