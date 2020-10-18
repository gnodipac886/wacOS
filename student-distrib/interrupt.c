#include "interrupt.h"
#include "x86_desc.h"

void __init_idt__(){
	int i;

	for(i = 0; i < 20; i++){
		if(i != 15){ 							// we want to skip the intel reserved ones
			idt[i].seg_selector = KERNEL_CS;
			idt[i].reserved4 = 0;
			idt[i].reserved3 = i == 2 ? 0 : 1; 	// #2 is an interrupt
			idt[i].reserved2 = 1;
			idt[i].reserved1 = 1;
			idt[i].size = 1;
			idt[i].reserved0 = 0;
			idt[i].dpl = 0;
			idt[i].present = 0;
		}
	}

	SET_IDT_ENTRY(idt[0x00], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x01], divide_error); 			// reserved
	SET_IDT_ENTRY(idt[0x02], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x03], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x04], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x05], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x06], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x07], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x08], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x09], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x0A], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x0B], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x0C], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x0D], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x0E], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x0F], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x10], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x11], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x12], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[0x13], divide_error); 			// divide by zero error
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