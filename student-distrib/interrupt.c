#include "interrupt.h"
#include "x86_desc.h"

void __init_idt__(){
	SET_IDT_ENTRY(idt[0x00], divide_error()); 			// divide by zero error
}

void divide_error(){
	printf("Exception: divide error");
	exception();
}

void exception() {
	//disable interrupts
	//squash(?) user-level program
	//return control to shell

	while (1) {
		printf("\n");
	}
}