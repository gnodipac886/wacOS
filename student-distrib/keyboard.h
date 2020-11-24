#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KB_PORT 0x60 // IO port for keyboard
#define KB_IRQ 	0x01 // keyboard interrupt

#define BUFFER_RS 0xFF // keyboard buffer overrun

// #ifndef ASM
// initialization of the keyboard for interrupt
void __keyboard_init__();
// takes in the input and display to screen
void handle_keyboard_interrupt();
// returns ptr to keyboard buffer
int get_kb_buf(char* buf);
void clear_terminal_buf();
void clear_kb_buf();

void handle_backspace();
void handle_enter();
void set_terminal_read_flag(int flag);
void terminal_switch(int terminal_num);
// #endif /* ASM */

#endif /* _KEYBOARD_H */
