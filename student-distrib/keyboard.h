#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KB_PORT 0x60 // IO port for keyboard
#define KB_IRQ 	0x01 // keyboard interrupt

#define BUFFER_RS 0xFF // keyboard buffer overrun

// #ifndef ASM
// initialization of the keyboard for interrupt
void __init_keyboard__();
// takes in the input and display to screen
void handle_keyboard_interrupt();
// returns ptr to keyboard buffer
int get_kb_buf(char* buf, int ter_num);
void clear_terminal_buf(int ter_num);
void clear_kb_buf(int ter_num);

void handle_backspace();
void handle_enter();
void set_terminal_read_flag(int flag);
void terminal_switch(int ter_num);
void terminal_switch_setup(int ter_num);
int get_curr_screen();

// #endif /* ASM */

#endif /* _KEYBOARD_H */
