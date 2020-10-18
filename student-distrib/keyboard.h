#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KB_IRQ 	0x01

extern void __keyboard_init__();
extern void handle_keyboard_interrupt();

#endif /* _KEYBOARD_H */
