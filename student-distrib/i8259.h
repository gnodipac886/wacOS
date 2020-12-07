/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20
#define SLAVE_8259_PORT     0xA0

#define MASTER_CMD          MASTER_8259_PORT
#define MASTER_DATA         MASTER_8259_PORT + 1

#define SLAVE_CMD           SLAVE_8259_PORT
#define SLAVE_DATA          SLAVE_8259_PORT + 1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

#define IRQ0_PIC			0x00
#define IRQ2_PIC			0x02
#define IRQ15_PIC 			0x0F

#define INIT_MASK_ALL 		0xFF
#define UNMASK_IRQ2			0xFB
#define SLAVE_IRQ_NUM       0x08

/* Externally-visible functions */

/* Initialize both PICs */
void __init_i8259__(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

extern void test_interrupts(void);

#endif /* _I8259_H */
