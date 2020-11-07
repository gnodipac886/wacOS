#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

#define BUF_SIZE 			128			//buffer can contain 128 chars
/*Scan Code Set 1*/
#define TAB_PRESSED			0x0F		//scan code for tab key
#define BACKSPACE_PRESSED	0x0E		//scan code for backspace key
#define ENTER_PRESSED		0x1C		//scan code for enter key
#define CTRL_PRESSED		0x1D		//scan code for left control key; also for right control's second byte
#define LSHIFT_PRESSED		0x2A		//scan code for left shift key
#define RSHIFT_PRESSED		0x36		//scan code for right shift key
#define ALT_PRESSED			0x38		//scan codee for left alt key; also right alt's second byte
#define CAPSLOCK_PRESSED	0x3A		//scan code for capslock key
#define RIGHT_KEY_BYTE		0xE0		//RCTRL, RALT first byte

#define SPACE_PRESSED		0x39		//scan code for space key

#define RELEASED_OFFSET		0x80		//'release' scan code offset from 'pressed'
#define ROW0_OFFSET_MAP		0x2			//offset to map scan codes corresponding to row0 array
#define ROW1_OFFSET_MAP		0x10		//offset to map scan codes corresponding to row1 array		
#define ROW2_OFFSET_MAP		0x1E		//offset to map scan codes corresponding to row2 array
#define ROW3_OFFSET_MAP		0x2B		//offset to map scan codes corresponding to row3 array

/*keyboard flags*/
int shift_flag = 0;
int capslock_flag = 0;
int ctrl_flag = 0;
int alt_flag = 0;

// 0x02 - 0x0D from 1 to =
char kb_sc_row0_nums[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '='};
char kb_sc_row0_shift_chars[] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+'};
// 0x10 - 0x1B from q to ]
char kb_sc_row1_let[] = {'q', 'w', 'e', 'r', 't', 'y', 'u','i', 'o', 'p', '[', ']'};
char kb_sc_row1_shift_chars[] = {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}'};
char kb_sc_row1_caps_chars[] = {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']'};
char kb_sc_row1_caps_shift[] = {'q', 'w', 'e', 'r', 't', 'y', 'u','i', 'o', 'p', '{', '}'};
// 0x1E - 0x29 from a to (backtick)
char kb_sc_row2_let[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`'};
char kb_sc_row2_shift_chars[] = {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~'};
char kb_sc_row2_caps_chars[] = {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`'};
char kb_sc_row2_caps_shift[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~'};
// 0x2B - 0x35 from \ (row 2) z(row 3) to /
char kb_sc_row3_let[] = {'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'};
char kb_sc_row3_shift_chars[] = {'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'};
char kb_sc_row3_caps_chars[] = {'\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'};
char kb_sc_row3_caps_shift[] = {'|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?'};

char buffer[BUF_SIZE];
char terminal_buf[BUF_SIZE];
int buffer_cur_idx;
int terminal_cur_idx;
int buffer_accessed_flag;				// 1 - being read/written to; 0 - not used (needed since terminal and keyboard driver both access it)

/* __keyboard_init__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initialization of the keyboard for interrupt
 *		Side Effects: none
 */
void __keyboard_init__(){
	enable_irq(KB_IRQ);
	buffer_cur_idx = 0;
	terminal_cur_idx = 0;
	buffer_accessed_flag = 0;
}

/* handle_keyboard_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Reads in keyboard and map input to the correct typed character
 *		Side Effects: none
 */
void handle_keyboard_interrupt(){
	cli();

	char kb_char = NULL;
	unsigned char keyboard_input = inb(KB_PORT);

	// conditions for different key presses
	if (keyboard_input == TAB_PRESSED) {
		kb_char = ' ';
	} else if (keyboard_input == BACKSPACE_PRESSED) {
		handle_backspace();
	} else if (keyboard_input == ENTER_PRESSED) {
		handle_enter();
	} else if (keyboard_input == CTRL_PRESSED) {		//update control flag
		ctrl_flag = 1;
	} else if (keyboard_input == (CTRL_PRESSED + RELEASED_OFFSET)) {
		ctrl_flag = 0;
	} else if (keyboard_input == LSHIFT_PRESSED) {		//update shift flag
		shift_flag = 1;
	} else if (keyboard_input == LSHIFT_PRESSED + RELEASED_OFFSET) {
		shift_flag = 0;
	} else if (keyboard_input == RSHIFT_PRESSED) {
		shift_flag = 1;
	} else if (keyboard_input == RSHIFT_PRESSED + RELEASED_OFFSET) {
		shift_flag = 0;
	} else if (keyboard_input == ALT_PRESSED) {			//update alt flag
		alt_flag = 1;
	} else if (keyboard_input == ALT_PRESSED + RELEASED_OFFSET) {
		alt_flag = 0;
	} else if (keyboard_input == CAPSLOCK_PRESSED) {	//update capslock flag
		capslock_flag = capslock_flag ^ 1;
	} else if (keyboard_input == RIGHT_KEY_BYTE) {
		switch(inb(KB_PORT)) {							//read secondbyte
			case CTRL_PRESSED:
				ctrl_flag = 1;
				break;
			case ALT_PRESSED:
				alt_flag = 1;
				break;
			case CTRL_PRESSED + RELEASED_OFFSET:
				ctrl_flag = 0;
				break;
			case ALT_PRESSED + RELEASED_OFFSET:
				alt_flag = 0;
				break;
			default:
				break;
		}
	} else if(keyboard_input == SPACE_PRESSED){					// space pressed = 0x39
		kb_char = ' ';
	} else if((keyboard_input <= 0x35) && (keyboard_input > 0x01)){
	// between 0x35 = /, 0x01 = esc on keyboard

		if(keyboard_input <= 0x0D && keyboard_input >= ROW0_OFFSET_MAP){
			// between 0x02 = 1 and 0x0D = "="				
			if (shift_flag) {															//deal with shift-related chars
				kb_char = kb_sc_row0_shift_chars[keyboard_input - ROW0_OFFSET_MAP]; 		
			} else {
				kb_char = kb_sc_row0_nums[keyboard_input - ROW0_OFFSET_MAP];				
			}
		} else if(keyboard_input <= 0x1B && keyboard_input >= ROW1_OFFSET_MAP){
			// between 0x10 = q and 0x1B = ]
			if (shift_flag && capslock_flag) {
				kb_char = kb_sc_row1_caps_shift[keyboard_input - ROW1_OFFSET_MAP]; 	
			} else if(shift_flag){														//deal with shift-related chars
				kb_char = kb_sc_row1_shift_chars[keyboard_input - ROW1_OFFSET_MAP];	
			} else if(capslock_flag){
				kb_char = kb_sc_row1_caps_chars[keyboard_input - ROW1_OFFSET_MAP];		
			} else {
				kb_char = kb_sc_row1_let[keyboard_input - ROW1_OFFSET_MAP];			
			}
		} else if(keyboard_input <= 0x29 && keyboard_input >= ROW2_OFFSET_MAP){
			// between 0x1E = a and 0x29 =  `
			if (shift_flag && capslock_flag) {											//deal with shift-related chars
				kb_char = kb_sc_row2_caps_shift[keyboard_input - ROW2_OFFSET_MAP]; 	
			} else if (shift_flag) {													//deal with shift-related chars
				kb_char = kb_sc_row2_shift_chars[keyboard_input - ROW2_OFFSET_MAP]; 	
			} else if(capslock_flag){
				kb_char = kb_sc_row2_caps_chars[keyboard_input - ROW2_OFFSET_MAP];		
			} else {
				kb_char = kb_sc_row2_let[keyboard_input - ROW2_OFFSET_MAP];			
			}
		} else if(keyboard_input <= 0x35 && keyboard_input >= ROW3_OFFSET_MAP){
			// between 0x2B = \ and 0x35 = /
			if (shift_flag && capslock_flag) {
				kb_char = kb_sc_row3_caps_shift[keyboard_input - ROW3_OFFSET_MAP]; 	
			} else if (shift_flag) {													//deal with shift-related chars
				kb_char = kb_sc_row3_shift_chars[keyboard_input - ROW3_OFFSET_MAP]; 	
			} else if(capslock_flag){
				kb_char = kb_sc_row3_caps_chars[keyboard_input - ROW3_OFFSET_MAP];		
			} else {
				kb_char = kb_sc_row3_let[keyboard_input - ROW3_OFFSET_MAP];			
			}
		} else{
			send_eoi(KB_IRQ);
			sti();
			return;
		}
	} else {
		send_eoi(KB_IRQ);
		sti();
		return;
	}


	if (kb_char != '\0') {
		if ((kb_char == 'L' || kb_char == 'l') && ctrl_flag == 1) {		//check ctrl+l or ctrl+L
			clear();
			update_cursor(0,0);											// move cursor to the top left
			int i;
			for (i = 0; i < buffer_cur_idx; i++) {						//print keyboard buffer to keep/maintain state before ctrl+l
				putc(buffer[i]);
			}
		} else if (buffer_cur_idx < BUF_SIZE -1 && buffer_accessed_flag == 0) {	//if keyboard buffer not filled (127 chars, last char is '\n')
			putc(kb_char);												//prints char to screen and updates cursor
			// while (buffer_accessed_flag == 1);						//wait till terminal finishes clearing buffer
			buffer[buffer_cur_idx] = kb_char;							//add char to keyboard buffer
			buffer_cur_idx++;

			// update the temporary buffer used by terminal driver
			terminal_buf[terminal_cur_idx] = kb_char;
			terminal_cur_idx++;
		}
	}

	send_eoi(KB_IRQ);
	return;
}

/* get_kb_buf
 *		Description: copies keyboard buffer into terminal's buffer
 * 		Inputs: buf = terminal buffer ptr
 * 		Return Value: index of last char in buffer
 */
int get_kb_buf(char* buf) {
	if(terminal_cur_idx >= 0 && terminal_cur_idx <= BUF_SIZE){
		memcpy((void*)buf, (void*)terminal_buf, terminal_cur_idx);

		// return 0 when its empty
		return terminal_cur_idx - 1 < 0 ? 0 : terminal_cur_idx - 1;
	} else{
		clear_terminal_buf();
		memcpy((void*)buf, (void*)terminal_buf, terminal_cur_idx);
		return 0;
	}
}

/* clear_terminal_buf
 *		Description: clears the temporary buffer used by terminal driver
 * 		Inputs: none
 * 		Return Value: none
 */
void clear_terminal_buf() {
	terminal_cur_idx = 0;									// reset the terminal index
	memset(terminal_buf, '\0', BUF_SIZE);
}

/* clear_kb_buf
 *		Description: clears the keyboard buffer
 * 		Inputs: none
 * 		Return Value: none
 */
void clear_kb_buf() {
	buffer_cur_idx = 0;										// reset the terminal index
	memset(buffer, '\0', BUF_SIZE);
}

/* handle_backspace
 *		Description: Updates kb buffer when backspace is pressed and change the display
 * 		Inputs: none
 * 		Return Value: none
 */
void handle_backspace() {
	// check to see if buffer is not empty
	if (buffer_cur_idx > 0 && terminal_cur_idx > 0) {
		buffer_cur_idx--;
		buffer[buffer_cur_idx] = '\0';						// backspace or null

		terminal_cur_idx--;
		terminal_buf[terminal_cur_idx] = '\0';				// backspace or null

		vid_backspace();
	}
}

/* handle_enter
 *		Description: Updates the buffers and change what is displayed when enter is pressed
 * 		Inputs: none
 * 		Return Value: none
 */
void handle_enter() {
	if (buffer_accessed_flag == 0) {				//ignore enters when handling enters
		buffer_accessed_flag = 1;					// enable flag

		buffer[buffer_cur_idx] = '\n';				//which may cause chars to be added while buffer_cur_idx = 0
		buffer_cur_idx++;

		terminal_buf[terminal_cur_idx] = '\n';		//which may cause chars to be added while buffer_cur_idx = 0
		terminal_cur_idx++;

		vid_enter();
		clear_kb_buf();
		buffer_accessed_flag = 0;					//reset flag
	}
}
