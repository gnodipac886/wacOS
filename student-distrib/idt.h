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

#define SYS_CALL 	0x80  /*system call interrupt*/
#define IRQ0_IDT	0x20   /*IRQ0 port for IDT*/
#define IRQ1_IDT	0x21   /* IRQ1 port for IDT*/
#define IRQ8_IDT  0x28    /*rtc IR line*/
#define IRQ12_IDT  0x2C    /*mouse IR line*/
#define IRQ15_IDT 	0x2F /*IRQ15 port for IDT*/

/*
void divide_error();
void reserv();
void nmi_interrupt();
void breakp();
void overflow();
void bounds_range_ex();
void invalid_op();
void dev_not_avail();
void double_fault();
void invalid_tss();
void seg_not_pres();
void stack_seg_fault();
void gen_prot_fault();
void page_fault();
void x87_fpu_fault();
void align_check();
void mach_check();
void simd_float_exc();
void system_call_handler();
void interrupt_dummy();
*/

/*
void divide_error();
void reserv();
void nmi_interrupt();
void breakp();
void overflow();
void bounds_range_ex();
void invalid_op();
void dev_not_avail();
void double_fault();
void invalid_tss();
void seg_not_pres();
void stack_seg_fault();
void gen_prot_fault();
void page_fault();
void x87_fpu_fault();
void align_check();
void mach_check();
void simd_float_exc();
void system_call_handler();
void interrupt_dummy();
*/

#ifndef ASM
/* initialization for IDT */
extern void __init_idt__();
#endif /* ASM */

#endif /* _INTERRUPT_H */
