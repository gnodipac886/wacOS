#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

#define BUF_SZ      128 //max size of buffer = 128 bytes
#define NUM_COLS    80 // video cols
#define NUM_ROWS    25 // video rows

/* initializes terminal */
int32_t terminal_open();
/* clears the terminal */
int32_t terminal_close();
/* reads from keyboard or input */
int32_t terminal_read(int32_t fd, const void* buf, int32_t nbytes);
/* writes to the screen */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* clears the terminal's buffer */
void clear_terminal_buf(void* buf);

#endif /*_TERMINAL_H */
