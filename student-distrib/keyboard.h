#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KB_PORT 0x60 // IO port for keyboard
#define KB_IRQ 	0x01 // keyboard interrupt

#define BUFFER_RS 0xFF // keyboard buffer overrun
#define UP        -1   // shell arrow up
#define DOWN       1   // shell arrow down
#define HIST_MX    10  // max commands to store for shell history buffer

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
void handle_esc();

void _init_shell_hist();
void shift_cmds();
void get_hist_buf(char* cmd);
void shell_prev_cmds(int arrow);

void set_terminal_read_flag(int flag);
// #endif /* ASM */

#endif /* _KEYBOARD_H */
