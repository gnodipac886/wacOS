/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    /* call spin lock (uncomment in the future)
    unsigned long flags;
    spin_lock_irqsave(&i8259_lock, flags);
    */

    // save mask
    uint32_t _master_mask_save, _slave_mask_save;
    _master_mask_save = inb(MASTER_8259_PORT+1);
    _slave_mask_save = inb(SLAVE_8259_PORT+1);

    outb(ICW1, MASTER_8259_PORT);   // Starts initialization ICW1 (cascade mode)
    outb(ICW1, SLAVE_8259_PORT);
 
    outb(ICW2_MASTER, MASTER_8259_PORT+1); // ICW2_MASTER: Master PIC vector offset
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1);   // ICW2_SLAVE: SLAVE PIC vector offset
    
    outb(ICW3_MASTER, MASTER_8259_PORT+1);  // ICW3_MASTER: Tell Master PIC the slave PIC is on IRQ2_PIC
    outb(ICW3_SLAVE, SLAVE_8259_PORT+1);   // ICW3_SLAVE: Tell Slave PIC its a slave on the Master PIC IRQ2_PIC.
    
    /* ICW4:
     *  Special fully nested mode or non special fully nested mod
     *  Buffered mode or non buffered mode
     *  Automatic EOI or Normal EOI
     */ 
    outb(ICW4, MASTER_8259_PORT+1);
    outb(ICW4, SLAVE_8259_PORT+1);
    
    /* restore saved mask */
    outb(_master_mask_save, MASTER_8259_PORT+1);
    outb(_slave_mask_save, SLAVE_8259_PORT+1);

    /* release spin lock (uncomment later)
    spin_unlock_irqrestore(&i8259_lock, flags);
     */
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    if(irq_num < IRQ0_PIC || irq_num > IRQ15_PIC){
        return;
    }
    /* bit "x" is set to 0 if pin "x" (master or slave PIC) is enabled*/
    uint8_t x;
    if(irq_num>=8){ //irq is 8~15, which is a slave mask.
        x = irq_num-8;
        slave_mask = slave_mask & ~(0x1<<x);
        //outb()          // to do
        outb(slave_mask, SLAVE_8259_PORT+1);
    }else{      //irq is 0~8, which is a master mask
        x = irq_num;
        master_mask = master_mask & ~(0x1<<x);
        outb(master_mask, MASTER_8259_PORT+1);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    if(irq_num < IRQ0_PIC || irq_num > IRQ15_PIC){
        return;
    }
    /* bit "x" is set to 1 if pin "x" (master or slave PIC) is enabled*/
    uint8_t x;
    if(irq_num>=8){ //irq is 8~15, which is a slave mask.
        x = irq_num-8;
        slave_mask = slave_mask | (0x1<<x);
        outb(slave_mask, SLAVE_8259_PORT+1);
        // here too
    }else{      //irq is 0~8, which is a master mask
        x = irq_num;
        master_mask = master_mask | (0x1<<x);
        outb(master_mask, MASTER_8259_PORT+1);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num < IRQ0_PIC || irq_num > IRQ15_PIC){
        return;
    }
    /*if IRQ came from slave, then send EOI signal to both master and slave.
      else only send EOI to master.*/
    if(irq_num >= 8){ // if IRQ came from slave, then send EOI signal to slave.
        outb((irq_num - 8) | EOI, SLAVE_8259_PORT);
        outb(IRQ2_PIC | EOI, MASTER_8259_PORT);
    }
    else{
        outb(irq_num | EOI, MASTER_8259_PORT); // Send EOI to master.
    }
}   
