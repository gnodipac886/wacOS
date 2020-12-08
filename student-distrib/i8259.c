/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* i8259_init 
 *	  Inputs: None
 *	  Return Value: None
 *	  Function: Initialize the i8259 PIC. Load ICW, setup master/slave pic.s
 *	  Side Effects: none	 
 */
void __init_i8259__(void) {

	// save mask
	// uint32_t _master_mask_save, _slave_mask_save;
	// _master_mask_save = inb(MASTER_DATA);
	// _slave_mask_save = inb(SLAVE_DATA);
	master_mask = INIT_MASK_ALL;		  	// Initialize mask to 0xFF
	slave_mask = INIT_MASK_ALL;		   		// Initialize mask to 0xFF

	outb(ICW1, MASTER_CMD);   				// Starts initialization ICW1 (cascade mode)
	outb(ICW1, SLAVE_CMD);

	outb(ICW2_MASTER, MASTER_DATA); 		// ICW2_MASTER: Master PIC vector offset
	outb(ICW2_SLAVE, SLAVE_DATA);   		// ICW2_SLAVE: SLAVE PIC vector offset

	outb(ICW3_MASTER, MASTER_DATA);  		// ICW3_MASTER: Tell Master PIC the slave PIC is on IRQ2_PIC
	outb(ICW3_SLAVE, SLAVE_DATA);   		// ICW3_SLAVE: Tell Slave PIC its a slave on the Master PIC IRQ2_PIC.

	/* ICW4:
	 *  Special fully nested mode or non special fully nested mode
	 *  Buffered mode or non buffered mode
	 *  Automatic EOI or Normal EOI
	 */
	outb(ICW4, MASTER_DATA);
	outb(ICW4, SLAVE_DATA);

	/* restore saved mask */
	outb(master_mask, MASTER_DATA);
	outb(slave_mask, SLAVE_DATA);

}

/* enable_irq 
 *	  Inputs: IRQ number
 *	  Return Value: None
 *	  Function: Enable (unmask) the specified IRQ
 *	  Side Effects: none	 
 */
void enable_irq(uint32_t irq_num) {
	if(irq_num < IRQ0_PIC || irq_num > IRQ15_PIC){
		return;
	}
	/* bit "x" is set to 0 if pin "x" (master or slave PIC) is enabled*/
	uint8_t x;
	if(irq_num >= SLAVE_IRQ_NUM){ 		//irq is 8~15, which is a slave mask.
		x = irq_num-SLAVE_IRQ_NUM;
		slave_mask &= ~(0x1 << x);
		outb(slave_mask, SLAVE_DATA);

		master_mask &= UNMASK_IRQ2;
		outb(master_mask, MASTER_DATA);
	}else{	  							//irq is 0~8, which is a master mask
		x = irq_num;
		master_mask &= ~(0x1 << x);
		outb(master_mask, MASTER_DATA);
	}
}

/* disable_irq 
 *	  Inputs: IRQ number
 *	  Return Value: None
 *	  Function: Disable (mask) the specified IRQ
 *	  Side Effects: none	 
 */
void disable_irq(uint32_t irq_num) {
	if(irq_num < IRQ0_PIC || irq_num > IRQ15_PIC){
		return;
	}
	/* bit "x" is set to 1 if pin "x" (master or slave PIC) is enabled*/
	uint8_t x;
	if(irq_num>=SLAVE_IRQ_NUM){ 		// irq is 8~15, which is a slave mask.
		x = irq_num-SLAVE_IRQ_NUM;
		slave_mask |= (0x1<<x);			// left shift 1 bit to mask that IRQ
		outb(slave_mask, SLAVE_DATA);

		master_mask |= ~UNMASK_IRQ2;
		outb(master_mask, MASTER_DATA);
	}else{	  //irq is 0~8, which is a master mask
		x = irq_num;
		master_mask |= (0x1<<x);		// left shift 1 bit to mask that IRQ out
		outb(master_mask, MASTER_DATA);
	}
}

/* send_eoi 
 *	  Inputs: IRQ number
 *	  Return Value: None
 *	  Function: Send end-of-interrupt signal for the specified IRQ
 *	  Side Effects: none	 
 */
void send_eoi(uint32_t irq_num) {
	if(irq_num < IRQ0_PIC || irq_num > IRQ15_PIC){
		return;
	}
	/*if IRQ came from slave, then send EOI signal to both master and slave.
	  else only send EOI to master.*/
	if(irq_num >= SLAVE_IRQ_NUM){ 		// if IRQ came from slave, then send EOI signal to slave.
		outb((irq_num - SLAVE_IRQ_NUM) | EOI, SLAVE_CMD);
		outb(IRQ2_PIC | EOI, MASTER_CMD);
	}
	else{
		outb(irq_num | EOI, MASTER_CMD); // Send EOI to master.
	}
}
