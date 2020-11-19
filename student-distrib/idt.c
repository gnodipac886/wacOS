#include "idt.h"
#include "assembly_linkage.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
// functions from assembly_linkage
extern void keyboard_interrupt_stub();
extern void rtc_interrupt_stub();
extern void mouse_interrupt_stub();
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
	// while (1);
}

/* divide_error
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void divide_error(){
	//cli();
	// clear();
	printf("Exception: Divides 0 error\n");
	exception();
	// sti();
}

/* reserv
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void reserv(){
	//cli();
	// clear();
	printf("Exception: Reserved\n");
	exception();
	// sti();
}

/* nmi_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void nmi_interrupt(){
	//cli();
	// clear();
	printf("Exception: NMI Interrupt\n");
	exception();
	// sti();
}

/* breakp
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void breakp(){
	//cli();
	// clear();
	printf("Exception: Breakpoint\n");
	exception();
	// sti();
}

/* overflow
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void overflow(){
	//cli();
	// clear();
	printf("Exception: Overflow\n");
	exception();
	// sti();
}

/* bounds_range_ex
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void bounds_range_ex(){
	//cli();
	// clear();
	printf("Exception: Bounds range exceeded\n");
	exception();
	// sti();
}

/* invalid_op
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void invalid_op(){
	//cli();
	// clear();
	printf("Exception: Invalid opcode\n");
	exception();
	// sti();
}

/* dev_not_avail
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void dev_not_avail(){
	//cli();
	// clear();
	printf("Exception: Device not available\n");
	exception();
	// sti();
}

/* double_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void double_fault(){
	//cli();
	// clear();
	printf("Exception: Double fault\n");
	exception();
	// sti();
}

/* invalid_tss
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void invalid_tss(){
	//cli();
	// clear();
	printf("Exception: Invalid TSS\n");
	exception();
	// sti();
}

/* seg_not_pres
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void seg_not_pres(){
	//cli();
	// clear();
	printf("Exception: Segment not present\n");
	exception();
	// sti();
}

/* stack_seg_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void stack_seg_fault(){
	//cli();
	// clear();
	printf("Exception: Stack-segment fault\n");
	exception();
	// sti();
}

/* gen_prot_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void gen_prot_fault(){
	//cli();
	// clear();
	printf("Exception: General protection fault\n");
	exception();
	// sti();
}

/* page_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void page_fault(){
	//cli();
	// clear();
	printf("Exception: Page fault\n");
	exception();
	// sti();
}

/* x87_fpu_fault
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void x87_fpu_fault(){
	//cli();
	// clear();
	printf("Exception: x87 FPU error\n");
	exception();
	// sti();
}

/* align_check
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void align_check(){
	//cli();
	// clear();
	printf("Exception: Alignment check\n");
	exception();
	// sti();
}

/* mach_check
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void mach_check(){
	//cli();
	// clear();
	printf("Exception: Machine check\n");
	exception();
	// sti();
}

/* simd_float_exc
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void simd_float_exc(){
	//cli();
	// clear();
	printf("Exception: SIMD Floating-Point Exception\n");
	exception();
	// sti();
}

/* system_call_handler
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void system_call_handler() {
	//hard interrupts have higher priority
	// sti();
	// clear();
	printf("System call was called");
}

/* interrupt_dummy
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Clears screen and prints out the exception eror
 *		Side Effects: Halts everything
 */
void interrupt_dummy(){
	//cli();
	// clear();
	printf("You haven't set up this interrupt yet\n");
	// sti();
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
		if (i == 9 || i == 15 || (i >= 20 && i <= 31)) { 			// 9, 15, 20, 31 are intel reserved trap vectors
			continue;
		}
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0;		// reserved to 0
		idt[i].reserved3 = 0; 	// i == SYS_CALL ? 1 : 0;
		idt[i].reserved2 = 1; 	// i == SYS_CALL ? 0 : 1;
		idt[i].reserved1 = 1;		// reserved to 1
		idt[i].size = 1; 		// i == SYS_CALL ? 0 : 1;
		idt[i].reserved0 = 0; // reserved to 0
		idt[i].dpl = i == SYS_CALL ? 0x3 : 0x0; // SYS_CALL = 0x03, interrupts = 0x0
		idt[i].present = 1; // valid interrupt descriptor
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

	// SET_IDT_ENTRY(idt[DE], squash_user_exception); 			// divide by zero error
	// SET_IDT_ENTRY(idt[DB], squash_user_exception); 				// reserved
	// SET_IDT_ENTRY(idt[MI], squash_user_exception); 			// NMI Interrupt........................
	// SET_IDT_ENTRY(idt[BP], squash_user_exception); 				// Breakpoint
	// SET_IDT_ENTRY(idt[OF], squash_user_exception); 				// Overflow
	// SET_IDT_ENTRY(idt[BR], squash_user_exception); 		// Bounds range exceeded
	// SET_IDT_ENTRY(idt[UD], squash_user_exception); 			// Invalid opcode
	// SET_IDT_ENTRY(idt[NM], squash_user_exception); 			// Device not available
	// SET_IDT_ENTRY(idt[DF], squash_user_exception); 			// Double fault
	// SET_IDT_ENTRY(idt[TS], squash_user_exception); 			// Invalid TSS
	// SET_IDT_ENTRY(idt[NP], squash_user_exception); 			// Segment not present
	// SET_IDT_ENTRY(idt[SS], squash_user_exception); 		// Stack-segment fault
	// SET_IDT_ENTRY(idt[GP], squash_user_exception); 		// General protection fault
	// SET_IDT_ENTRY(idt[PF], squash_user_exception); 			// Page fault
	// SET_IDT_ENTRY(idt[MF], squash_user_exception); 			// x87 FPU error
	// SET_IDT_ENTRY(idt[AC], squash_user_exception); 			// Alignment check
	// SET_IDT_ENTRY(idt[MC], squash_user_exception); 			// Machine check
	// SET_IDT_ENTRY(idt[XF], squash_user_exception); 		// SIMD Floating-Point Exception

	SET_IDT_ENTRY(idt[IRQ1_IDT], keyboard_interrupt_stub); 		// handle keyboard interrupt
	SET_IDT_ENTRY(idt[IRQ8_IDT], rtc_interrupt_stub); 		// handle rtc interrupt
	SET_IDT_ENTRY(idt[IRQ12_IDT], mouse_interrupt_stub);	//handle system call
	SET_IDT_ENTRY(idt[SYS_CALL], system_call_interrupt);	//handle system call
}
