#include "interrupt.h"
#include "x86_desc.h"
#include "lib.h"

void exception() {
	//disable interrupts
	//squash(?) user-level program
	//return control to shell

	while (1);
}

void divide_error(){
	cli();
	printf("Exception: divide error\n");
	exception();
}

void reserv(){
	cli();
	printf("Exception: Reserved\n");
	exception();
}

void nmi_interrupt(){
	cli();
	printf("Exception: NMI Interrupt\n");
	exception();
}

void breakp(){
	cli();
	printf("Exception: Breakpoint\n");
	exception();
}

void overflow(){
	cli();
	printf("Exception: Overflow\n");
	exception();
}

void bounds_range_ex(){
	cli();
	printf("Exception: Bounds range exceeded\n");
	exception();
}

void invalid_op(){
	cli();
	printf("Exception: Invalid opcode\n");
	exception();
}

void dev_not_avail(){
	cli();
	printf("Exception: Device not available\n");
	exception();
}

void double_fault(){
	cli();
	printf("Exception: Double fault\n");
	exception();
}

void invalid_tss(){
	cli();
	printf("Exception: Invalid TSS\n");
	exception();
}

void seg_not_pres(){
	cli();
	printf("Exception: Segment not present\n");
	exception();
}

void stack_seg_fault(){
	cli();
	printf("Exception: Stack-segment fault\n");
	exception();
}

void gen_prot_fault(){
	cli();
	printf("Exception: General protection fault\n");
	exception();
}

void page_fault(){
	cli();
	printf("Exception: Page fault\n");
	exception();
}

void x87_fpu_fault(){
	cli();
	printf("Exception: x87 FPU error\n");
	exception();
}

void align_check(){
	cli();
	printf("Exception: Alignment check\n");
	exception();
}

void mach_check(){
	cli();
	printf("Exception: Machine check\n");
	exception();
}

void simd_float_exc(){
	cli();
	printf("Exception: SIMD Floating-Point Exception\n");
	exception();
}

void system_call_handler() {
	//hard interrupts have higher priority
	printf("System call was called");
}


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
			idt[i].present = 1;
		}
	}
}



void system_call_handler() {
	//hard interrupts have higher priority
	printf("System call was called");
}

void __init_idt__(){
	int i;

	//initialize trap-gates
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
			idt[i].present = 1;
		}
	}

	SET_IDT_ENTRY(idt[DE], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[DB], reserv); 						// reserved
	SET_IDT_ENTRY(idt[BP], breakp); 						// Breakpoint
	SET_IDT_ENTRY(idt[OF], overflow); 					// Overflow
	SET_IDT_ENTRY(idt[BR], bounds_range_ex); 		// Bounds range exceeded
	SET_IDT_ENTRY(idt[UD], invalid_op); 				// Invalid opcode
	SET_IDT_ENTRY(idt[NM], dev_not_avail); 			// Device not available
	SET_IDT_ENTRY(idt[DF], double_fault); 			// Double fault
	SET_IDT_ENTRY(idt[TS], invalid_tss); 				// Invalid TSS
	SET_IDT_ENTRY(idt[NP], seg_not_pres); 			// Segment not present
	SET_IDT_ENTRY(idt[SS], stack_seg_fault); 		// Stack-segment fault
	SET_IDT_ENTRY(idt[GP], gen_prot_fault); 		// General protection fault
	SET_IDT_ENTRY(idt[PF], page_fault); 				// Page fault
	SET_IDT_ENTRY(idt[MF], x87_fpu_fault); 			// x87 FPU error
	SET_IDT_ENTRY(idt[AC], align_check); 				// Alignment check
	SET_IDT_ENTRY(idt[MC], mach_check); 				// Machine check
	SET_IDT_ENTRY(idt[XF], simd_float_exc); 		// SIMD Floating-Point Exception

<<<<<<< HEAD
=======

>>>>>>> origin/ericd3
	//initialize interrupt-gates
	for(i = 32; i < NUM_VEC; i++){				// all non-reserved
		if(i != 0x80){ 							
			idt[i].seg_selector = KERNEL_CS;
			idt[i].reserved4 = 0;
			idt[i].reserved3 = 0; 	
			idt[i].reserved2 = 1;
			idt[i].reserved1 = 1;
			idt[i].size = 1;
			idt[i].reserved0 = 0;
			idt[i].dpl = 0;
			idt[i].present = 0;

			SET_IDT_ENTRY(idt[i], NULL);

		} else {			//iniitalize task-gate
			idt[i].seg_selector = KERNEL_CS;
			//[i].reserved4 = 0;
			idt[i].reserved3 = 1; 	
			idt[i].reserved2 = 0;
			idt[i].reserved1 = 1;
			idt[i].size = 0;
			idt[i].reserved0 = 0;
			idt[i].dpl = 3;
			idt[i].present = 1;

			SET_IDT_ENTRY(idt[i], system_call_handler);
		}
	}
<<<<<<< HEAD

}
=======
}
>>>>>>> origin/ericd3
