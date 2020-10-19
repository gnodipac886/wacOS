#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

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
	y = 2 / i;
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
		"ud2;"
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
		"movb 	$127, 	%%al;"
		"addb 	$127, 	%%al;"
		"into;"
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
	int arr[2] = {1, 2};
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

// info mem

/* system_call_test
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Test for system call functionality
 *		Side Effects: none
 */
/*
<<<<<<< HEAD
int system_call_test()
=======
int system_call_test() {
>>>>>>> 9c6682e623d7506bee170819340d835f2e3478cd
	asm volatile(
		"movl $0x80, %%eax;"
		"call (%%eax);"
		: 								//no output operands yet
		:								//no input operands yet
<<<<<<< HEAD
		: "memory", "%eax", "%eip"		//clobbered registers
=======
		: "memory", "%eax", "%eip"		//clobbered registers	
>>>>>>> 9c6682e623d7506bee170819340d835f2e3478cd
	);

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
	ptr = NULL;
	x = *ptr;

	return 1;
}


/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("divide by 0 test", divide_error_test());
	// TEST_OUTPUT("invalid_opcode_test", invalid_opcode_test());
	// TEST_OUTPUT("overflow_test", overflow_test());
	// TEST_OUTPUT("bound range test", bound_range_test());

	// launch your tests here
}
