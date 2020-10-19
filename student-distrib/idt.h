#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#define DE 0x00     /*Divide Error*/
#define DB 0x01     /*RESERVED*/
#define MI 0x02 	/*NMI Interrupt*/
#define BP 0x03     /*Breakpoint*/
#define OF 0x04     /*Overflow*/
#define BR 0x05     /*BOUND Range Exceeded*/
#define UD 0x06     /*Invalid Opcode (Undefined Opcode)*/
#define NM 0x07     /*Device Not Available (No Math Coprocessor)*/
#define DF 0x08     /*Double Fault*/

#define TS 0x0A     /*Invalid TSS*/
#define NP 0x0B     /*Segment Not Present*/
#define SS 0x0C     /*Stack-Segment Fault*/
#define GP 0x0D     /*General Protection*/
#define PF 0x0E     /*Page Fault*/

#define MF 0x10     /*x87 FPU Floating-Point Error (Math Fault)*/
#define AC 0x11     /*Alignment Check*/
#define MC 0x12     /*Machine Check*/
#define XF 0x13     /*SIMD Floating-Point Exception*/

#define SYS_CALL 	0x80
#define IRQ0_IDT	0x20
#define IRQ1_IDT	0x21
#define IRQ8        0x28    /*rtc IR line*/
#define IRQ15_IDT 	0x2F

#ifndef ASM
extern void __init_idt__();
#endif /* ASM */

#endif /* _INTERRUPT_H */
