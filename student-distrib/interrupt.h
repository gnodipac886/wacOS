

#define DE 0x00     /*Divide Error*/
#define DB 0x01     /*RESERVED*/

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

#ifndef ASM
extern void __init_idt__();
#endif /* ASM */