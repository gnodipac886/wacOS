#include "idt.h"
#include "assembly_linkage.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "pit.h"
// functions from assembly_linkage
extern void keyboard_interrupt_stub();
extern void rtc_interrupt_stub();
extern void pit_interrupt_stub();
extern void system_call_interrupt();
extern void squash_user_exception();

/* exception
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Once reached an exception, kills everything else
 *		Side Effects: none
 */
void exception() {
	//disable interrupts
	//squash(?) user-level program
	//return control to shell
	squash_user_exception();
}

/* divide_error
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void divide_error(){
	printf("Exception: Divides 0 error\n");
	exception();
}

/* reserv
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void reserv(){
	printf("Exception: Reserved\n");
	exception();
}

/* nmi_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void nmi_interrupt(){
	printf("Exception: NMI Interrupt\n");
	exception();
}

/* breakp
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void breakp(){
	printf("Exception: Breakpoint\n");
	exception();
}

/* overflow
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void overflow(){
	printf("Exception: Overflow\n");
	exception();
}

/* bounds_range_ex
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void bounds_range_ex(){
	printf("Exception: Bounds range exceeded\n");
	exception();
}

/* invalid_op
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void invalid_op(){
	printf("Exception: Invalid opcode\n");
	exception();
}

/* dev_not_avail
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void dev_not_avail(){
	printf("Exception: Device not available\n");
	exception();
}

/* double_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void double_fault(){
	printf("Exception: Double fault\n");
	exception();
}

/* invalid_tss
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void invalid_tss(){
	printf("Exception: Invalid TSS\n");
	exception();
}

/* seg_not_pres
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void seg_not_pres(){
	printf("Exception: Segment not present\n");
	exception();
}

/* stack_seg_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void stack_seg_fault(){
	printf("Exception: Stack-segment fault\n");
	exception();
}

/* gen_prot_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void gen_prot_fault(){
	printf("Exception: General protection fault\n");
	exception();
}

/* page_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void page_fault(){
	printf("Exception: Page fault\n");
	exception();
}

/* x87_fpu_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void x87_fpu_fault(){
	printf("Exception: x87 FPU error\n");
	exception();
}

/* align_check
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void align_check(){
	printf("Exception: Alignment check\n");
	exception();
}

/* mach_check
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void mach_check(){
	printf("Exception: Machine check\n");
	exception();
}

/* simd_float_exc
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void simd_float_exc(){
	printf("Exception: SIMD Floating-Point Exception\n");
	exception();
}

/* system_call_handler
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void system_call_handler() {
	//hard interrupts have higher priority
	printf("System call was called");
}

/* interrupt_dummy
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void interrupt_dummy(){
	printf("You haven't set up this interrupt yet\n");

}

/* __init_idt__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initializing all IDT interrupt descriptors
 *		Side Effects: mapping the first 20 interrupts to the IDT
 */
void __init_idt__(){
	int i;

	// sets the interrupt descriptors, skips over 9, 15, and anything less than 31 - intel reserved
	for(i = 0; i < NUM_VEC; i++){
		if (i == 9 || i == 15 || (i >= 20 && i <= 31)) { 	// 9, 15, 20, 31 are intel reserved trap vectors
			continue;
		}
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0;								// reserved to 0
		idt[i].reserved3 = 0; 								// i == SYS_CALL ? 1 : 0;
		idt[i].reserved2 = 1; 								// i == SYS_CALL ? 0 : 1;
		idt[i].reserved1 = 1;								// reserved to 1
		idt[i].size = 1; 									// i == SYS_CALL ? 0 : 1;
		idt[i].reserved0 = 0; 								// reserved to 0
		idt[i].dpl = i == SYS_CALL ? 0x3 : 0x0; 			// SYS_CALL = 0x03, interrupts = 0x0
		idt[i].present = 1; 								// valid interrupt descriptor
	}

	SET_IDT_ENTRY(idt[DE], divide_error); 					// divide by zero error
	SET_IDT_ENTRY(idt[DB], reserv); 						// reserved
	SET_IDT_ENTRY(idt[MI], nmi_interrupt); 					// NMI Interrupt........................
	SET_IDT_ENTRY(idt[BP], breakp); 						// Breakpoint
	SET_IDT_ENTRY(idt[OF], overflow); 						// Overflow
	SET_IDT_ENTRY(idt[BR], bounds_range_ex); 				// Bounds range exceeded
	SET_IDT_ENTRY(idt[UD], invalid_op); 					// Invalid opcode
	SET_IDT_ENTRY(idt[NM], dev_not_avail); 					// Device not available
	SET_IDT_ENTRY(idt[DF], double_fault); 					// Double fault
	SET_IDT_ENTRY(idt[TS], invalid_tss); 					// Invalid TSS
	SET_IDT_ENTRY(idt[NP], seg_not_pres); 					// Segment not present
	SET_IDT_ENTRY(idt[SS], stack_seg_fault); 				// Stack-segment fault
	SET_IDT_ENTRY(idt[GP], gen_prot_fault); 				// General protection fault
	SET_IDT_ENTRY(idt[PF], page_fault); 					// Page fault
	SET_IDT_ENTRY(idt[MF], x87_fpu_fault); 					// x87 FPU error
	SET_IDT_ENTRY(idt[AC], align_check); 					// Alignment check
	SET_IDT_ENTRY(idt[MC], mach_check); 					// Machine check
	SET_IDT_ENTRY(idt[XF], simd_float_exc); 				// SIMD Floating-Point Exception

	SET_IDT_ENTRY(idt[IRQ0_IDT], pit_interrupt_stub);		//handle pit interrupt
	SET_IDT_ENTRY(idt[IRQ1_IDT], keyboard_interrupt_stub); 	// handle keyboard interrupt
	SET_IDT_ENTRY(idt[IRQ8_IDT], rtc_interrupt_stub); 		// handle rtc interrupt
	SET_IDT_ENTRY(idt[SYS_CALL], system_call_interrupt);	//handle system call
	
}
