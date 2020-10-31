#include "assembly_linkage.h"
#include "system_calls.h"
.globl keyboard_interrupt_stub, rtc_interrupt_stub

/* Interrupt Assembly Linkage
 * Description: x86 wrapper function that calls interrupt handlers;
                stubs - pushes NOT(IRQ#) for calculating interrupt_handler_jump table offset
 *              common_interrupt - saves/restores general purpose register values before/after interrupt
 * Input: None
 * Output: None
 * Side Effects: interrupt handler gets executed
 */

keyboard_interrupt_stub:
    pushl $-2                                    /*Negative num for keyboard IRQ1*/
    jmp common_interrupt

rtc_interrupt_stub:
    pushl $-9                                    /*Negative num for rtc IRQ8*/
    jmp common_interrupt

common_interrupt:
    pushal                                      /*save all register values before interrupt*/
    movl 32(%esp), %eax                        /*Get neg IRQ num on Kernel stack*/
    notl %eax                                   /*Negate for offset from IRQ0 Vector #*/
    call *interrupt_handler_jump(,%eax,4)
    popal
    addl $4, %esp
    iret                                       /*return from interrupt*/

interrupt_handler_jump:
    .long 0                                    /*handler ptrs for IR lines of Master PIC*/
    .long handle_keyboard_interrupt
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0

    .long handle_rtc_interrupt                  /*handler ptrs for IR lines of Slave PIC*/
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
 *              system_call_interrupt - saves/restores general purpose register values before/after interrupt
 * Input: None
 * Output: None
 * Side Effects: system call gets handled
 */

system_call_interrupt:
    pushal
    pushl %edx 
    pushl %ecx 
    pushl %ebx 
    call *system_call_jmp(,%eax,4)
    movl %eax, sys_call_ret_val
    popl %ebx 
    popl %ecx 
    popl %edx
    popal
    movl sys_call_ret_val, %eax
    iret

system_call_jump:
    .long 0                 //halt 
    .long 0                 //execute 
    .long read 
    .long write 
    .long open 
    .long close 
    .long 0                 //getargs 
    .long 0                 //vidmap
    .long 0                 //set_handler 
    .long 0                 //sigreturn

sys_call_ret_val:
    .long 0 
