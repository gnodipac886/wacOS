#include "assembly_linkage.h"
.globl keyboard_interrupt_stub, rtc_interrupt_stub

keyboard_interrupt_stub:
    pushl $-2                                    /*Negative num for keyboard IRQ1*/
    jmp common_interrupt

rtc_interrupt_stub:
    pushl $-9                                    /*Negative num for rtc IRQ8*/
    jmp common_interrupt

common_interrupt:
    pushal
    movl 32(%esp), %eax                        /*Get neg IRQ num on Kernel stack*/
    notl %eax                                   /*Negate for offset from IRQ0 Vector #*/
    call interrupt_handler_jump(,%eax,4)
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

    .long handle_rtc_interrupt               /*handler ptrs for IR lines of Slave PIC*/
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0

