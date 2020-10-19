#include "idt.h"
#include "assembly_linkage.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
extern void keyboard_interrupt_stub();
extern void rtc_interrupt_stub();
void exception() {
	//disable interrupts
	//squash(?) user-level program
	//return control to shell

	while (1);
}

void divide_error(){
	//cli();
	clear();
	printf("Exception: Divides 0 error\n");
	exception();
	sti();
}

void reserv(){
	//cli();
	clear();
	printf("Exception: Reserved\n");
	exception();
	sti();
}

void nmi_interrupt(){
	//cli();
	clear();
	printf("Exception: NMI Interrupt\n");
	exception();
	sti();
}

void breakp(){
	//cli();
	clear();
	printf("Exception: Breakpoint\n");
	exception();
	sti();
}

void overflow(){
	//cli();
	clear();
	printf("Exception: Overflow\n");
	exception();
	sti();
}

void bounds_range_ex(){
	//cli();
	clear();
	printf("Exception: Bounds range exceeded\n");
	exception();
	sti();
}

void invalid_op(){
	//cli();
	clear();
	printf("Exception: Invalid opcode\n");
	exception();
	sti();
}

void dev_not_avail(){
	//cli();
	clear();
	printf("Exception: Device not available\n");
	exception();
	sti();
}

void double_fault(){
	//cli();
	clear();
	printf("Exception: Double fault\n");
	exception();
	sti();
}

void invalid_tss(){
	//cli();
	clear();
	printf("Exception: Invalid TSS\n");
	exception();
	sti();
}

void seg_not_pres(){
	//cli();
	clear();
	printf("Exception: Segment not present\n");
	exception();
	sti();
}

void stack_seg_fault(){
	//cli();
	clear();
	printf("Exception: Stack-segment fault\n");
	exception();
	sti();
}

void gen_prot_fault(){
	//cli();
	clear();
	printf("Exception: General protection fault\n");
	exception();
	sti();
}

void page_fault(){
	//cli();
	clear();
	printf("Exception: Page fault\n");
	exception();
	sti();
}

void x87_fpu_fault(){
	//cli();
	clear();
	printf("Exception: x87 FPU error\n");
	exception();
	sti();
}

void align_check(){
	//cli();
	clear();
	printf("Exception: Alignment check\n");
	exception();
	sti();
}

void mach_check(){
	//cli();
	clear();
	printf("Exception: Machine check\n");
	exception();
	sti();
}

void simd_float_exc(){
	//cli();
	clear();
	printf("Exception: SIMD Floating-Point Exception\n");
	exception();
	sti();
}

void system_call_handler() {
	//hard interrupts have higher priority
	sti();
	clear();
	printf("System call was called");
}

void interrupt_dummy(){
	//cli();
	clear();
	printf("You haven't set up this interrupt yet\n");
	sti();
}

void __init_idt__(){
	int i;

	for(i = 0; i < NUM_VEC; i++){
		if (i == 9 || i == 15 || (i >= 20 && i <= 31)) {
			continue;
		}
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0;
		idt[i].reserved3 = 0; 	// i == SYS_CALL ? 1 : 0;
		idt[i].reserved2 = 1; 	// i == SYS_CALL ? 0 : 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1; 		// i == SYS_CALL ? 0 : 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = i == SYS_CALL ? 0x3 : 0x0;
		idt[i].present = 1;
	}

	SET_IDT_ENTRY(idt[DE], divide_error); 			// divide by zero error
	SET_IDT_ENTRY(idt[DB], reserv); 				// reserved
	SET_IDT_ENTRY(idt[MI], nmi_interrupt); 			// NMI Interrupt........................
	SET_IDT_ENTRY(idt[BP], breakp); 				// Breakpoint
	SET_IDT_ENTRY(idt[OF], overflow); 				// Overflow
	SET_IDT_ENTRY(idt[BR], bounds_range_ex); 		// Bounds range exceeded
	SET_IDT_ENTRY(idt[UD], invalid_op); 			// Invalid opcode
	SET_IDT_ENTRY(idt[NM], dev_not_avail); 			// Device not available
	SET_IDT_ENTRY(idt[DF], double_fault); 			// Double fault
	SET_IDT_ENTRY(idt[TS], invalid_tss); 			// Invalid TSS
	SET_IDT_ENTRY(idt[NP], seg_not_pres); 			// Segment not present
	SET_IDT_ENTRY(idt[SS], stack_seg_fault); 		// Stack-segment fault
	SET_IDT_ENTRY(idt[GP], gen_prot_fault); 		// General protection fault
	SET_IDT_ENTRY(idt[PF], page_fault); 			// Page fault
	SET_IDT_ENTRY(idt[MF], x87_fpu_fault); 			// x87 FPU error
	SET_IDT_ENTRY(idt[AC], align_check); 			// Alignment check
	SET_IDT_ENTRY(idt[MC], mach_check); 			// Machine check
	SET_IDT_ENTRY(idt[XF], simd_float_exc); 		// SIMD Floating-Point Exception
	SET_IDT_ENTRY(idt[IRQ1_IDT], keyboard_interrupt_stub); 		// handle keyboard interrupt
	SET_IDT_ENTRY(idt[IRQ8_IDT], rtc_interrupt_stub); 		// handle rtc interrupt
	SET_IDT_ENTRY(idt[SYS_CALL], system_call_handler);	//handle system call
}
