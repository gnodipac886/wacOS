#include "assembly_linkage.h"
.globl keyboard_interrupt_stub, rtc_interrupt_stub
.globl system_call_interrupt
.globl system_call_ret

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

common_interrupt:
	pushal									  /*save all register values before interrupt*/
	movl 32(%esp), %eax						/*Get neg IRQ num on Kernel stack*/
	notl %eax								   /*Negate for offset from IRQ0 Vector #*/
	call *interrupt_handler_jump(,%eax,4)
	popal
	addl $4, %esp
	iret									   /*return from interrupt*/

interrupt_handler_jump:
	.long 0									/*handler ptrs for IR lines of Master PIC*/
	.long handle_keyboard_interrupt
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0

	.long handle_rtc_interrupt				  /*handler ptrs for IR lines of Slave PIC*/
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0


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
	cmpl 	$0, 	%eax 					// have to check 0 case since its from 1 - 10
	je 		wrong_number
	cmpl	$10,	%eax					// check number for below 10
	jbe		valid_call_number

wrong_number:
	movl	$-1,	sys_call_ret_val	  //call number is not in 1-10 range
	jmp 	system_call_ret

valid_call_number:
	pushl   %edx 
	pushl   %ecx 
	pushl   %ebx 
	sti
	call	*system_call_jmp(,%eax,4)
	movl	%eax,   sys_call_ret_val
	popl	%ebx 
	popl	%ecx 
	popl	%edx

system_call_ret:
	popal
	movl 	sys_call_ret_val, %eax
	iret

system_call_jmp:
	.long 0
	.long halt
	.long execute
	.long read 
	.long write 
	.long open 
	.long close 
	.long 0				 //getargs 
	.long 0				 //vidmap
	.long 0				 //set_handler 
	.long 0				 //sigreturn

sys_call_ret_val:
	.long 0 
