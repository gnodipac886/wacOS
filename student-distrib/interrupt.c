#include "interrupt.h"
#include "x86_desc.h"
#include "lib.h"

void __init_idt__(){
	int i;

	for(i = 0; i < 20; i++){
		if(i != 9 || i != 15){ 							// we want to skip the intel reserved ones
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

	SET_IDT_ENTRY(idt[DE], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[DB], divide_error); 			// reserved
	SET_IDT_ENTRY(idt[BP], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[OF], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[BR], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[UD], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[NM], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[DF], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[TS], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[NP], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[SS], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[GP], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[PF], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[MF], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[AC], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[MC], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[XF], divide_error); 			// divide by zero error
}

void divide_error(){
	cli();
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