#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KB_PORT 0x60 // IO port for keyboard
#define KB_IRQ 	0x01 // keyboard interrupt

#define BUFFER_RS 0xFF // keyboard buffer overrun

#ifndef ASM
// initialization of the keyboard for interrupt
//void __keyboard_init__();
// takes in the input and display to screen
//void handle_keyboard_interrupt();
#endif /* ASM */

#endif /* _KEYBOARD_H */
