#include "assembly_linkage.h"
.globl keyboard_interrupt_stub, rtc_interrupt_stub, mouse_interrupt_stub
.globl system_call_interrupt, system_call_ret
.globl squash_user_exception, is_exception

/* Interrupt Assembly Linkage
 * Description: x86 wrapper function that calls interrupt handlers;
				stubs - pushes NOT(IRQ#) for calculating interrupt_handler_jump table offset
 *			  common_interrupt - saves/restores general purpose register values before/after interrupt
 * Input: None
 * Output: None
 * Side Effects: interrupt handler gets executed
 */

keyboard_interrupt_stub:
	pushl $-2									/*Negative num for keyboard IRQ1*/
	jmp common_interrupt

rtc_interrupt_stub:
	pushl $-9									/*Negative num for rtc IRQ8*/
	jmp common_interrupt

mouse_interrupt_stub:
	push $-13									/*Negative num for rtc IRQ12*/
	jmp common_interrupt

common_interrupt:
	pushal									  	/*save all register values before interrupt*/
	movl 32(%esp), %eax							/*Get neg IRQ num on Kernel stack*/
	notl %eax								   	/*Negate for offset from IRQ0 Vector #*/
	call *interrupt_handler_jump(,%eax,4)
	popal
	addl $4, %esp								/*pop off the irq number*/
	iret									   	/*return from interrupt*/

interrupt_handler_jump:
	.long 0										/*handler ptrs for IR lines of Master PIC*/
	.long handle_keyboard_interrupt
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0

	.long handle_rtc_interrupt				  	/*handler ptrs for IR lines of Slave PIC*/
	.long 0
	.long 0
	.long 0
	.long handle_mouse_interrupt
	.long 0
	.long 0
	.long 0




 /* squash_user_exception
 * Description: helper function for exceptions to jump to halt
 * Input: None
 * Output: None
 * Side Effects: sets 256 as return val of halt
 */

squash_user_exception:
	movl 	$1, 	%eax
	movb 	$255, 	%bl 						// 256 for exception
	movl 	$256, 	is_exception 				// indicate we have exception
	jmp 	call_syscall


/* System Call Assembly Linkage
 * Description: x86 wrapper function that calls interrupt handlers;
				eax - holds system call number
 *			  system_call_interrupt - saves/restores general purpose register values before/after interrupt
 * Input: None
 * Output: None
 * Side Effects: system call gets handled
 */

system_call_interrupt:
	pushal
	cmpl 	$0, 	%eax 						// have to check 0 case since its from 1 - 10
	je 		wrong_number
	cmpl	$10,	%eax						// check number for below 10
	jbe		valid_call_number

wrong_number:
	movl	$-1,	sys_call_ret_val	  		// call number is not in 1-10 range
	jmp 	system_call_ret

valid_call_number:
	pushl   %edx 								// push system call arguents
	pushl   %ecx 
	pushl   %ebx 
	
call_syscall:
	sti
	call	*system_call_jmp(,%eax,4)			// 4 for 4 bytes of memory
	movl	%eax,   sys_call_ret_val 			// save return value in the placeholder
	popl	%ebx 								// pop off the arguments
	popl	%ecx 
	popl	%edx

system_call_ret:
	popal
	movl 	sys_call_ret_val, %eax 				// get the return value from the placeholder
	movl 	$0, 	is_exception				// reset the exception flag
	iret

system_call_jmp:
	.long 0
	.long halt
	.long execute
	.long read 
	.long write 
	.long open 
	.long close 
	.long getargs 
	.long vidmap
	.long 0				 //set_handler 
	.long 0				 //sigreturn

sys_call_ret_val:
	.long 0 

is_exception:
	.long 0
