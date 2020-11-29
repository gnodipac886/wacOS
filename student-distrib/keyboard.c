#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "scheduler.h"
#include "paging.h"
#include "system_calls.h"

#define BUF_SIZE 			128			//buffer can contain 128 chars
#define MAX_TERMINALS		3			//we support maximum of 3 terminals
/*Scan Code Set 1*/
#define TAB_PRESSED			0x0F		//scan code for tab key
#define BACKSPACE_PRESSED	0x0E		//scan code for backspace key
#define ENTER_PRESSED		0x1C		//scan code for enter key
#define CTRL_PRESSED		0x1D		//scan code for left control key; also for right control's second byte
#define LSHIFT_PRESSED		0x2A		//scan code for left shift key
#define RSHIFT_PRESSED		0x36		//scan code for right shift key
#define ALT_PRESSED			0x38		//scan code for left alt key; also right alt's second byte
#define CAPSLOCK_PRESSED	0x3A		//scan code for capslock key
#define RIGHT_KEY_BYTE		0xE0		//RCTRL, RALT first byte

#define F1_PRESSED			0x3B		//scan code for f1 key
#define F2_PRESSED			0x3C		//scan code for f2 key
#define F3_PRESSED			0x3D		//scan code for f3 key

#define SPACE_PRESSED		0x39		//scan code for space key

#define RELEASED_OFFSET		0x80		//'release' scan code offset from 'pressed'
#define ROW0_OFFSET_MAP		0x2			//offset to map scan codes corresponding to row0 array
#define ROW1_OFFSET_MAP		0x10		//offset to map scan codes corresponding to row1 array
#define ROW2_OFFSET_MAP		0x1E		//offset to map scan codes corresponding to row2 array
#define ROW3_OFFSET_MAP		0x2B		//offset to map scan codes corresponding to row3 array

extern void squash_user_exception();

/*keyboard flags*/
int shift_flag = 0;
int capslock_flag = 0;
int ctrl_flag = 0;
int alt_flag = 0;

/*other flags*/
int terminal_read_flag[MAX_TERMINALS] = {0, 0, 0};

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

int curr_screen;
char input_bufs[MAX_TERMINALS][BUF_SIZE];	//array of input buffers for terminal 1-3
char ter_bufs[MAX_TERMINALS][BUF_SIZE]; 	// array of input buffers for terminals
char * terminal_buf;
char * buffer;

int input_indicies[MAX_TERMINALS];
int terminal_indicies[MAX_TERMINALS];
int buffer_accessed_flag;					// 1 - being read/written to; 0 - not used (needed since terminal and keyboard driver both access it)

/* __keyboard_init__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initialization of the keyboard for interrupt
 *		Side Effects: none
 */
void __init_keyboard__(){
	int i;
	enable_irq(KB_IRQ);
	buffer_accessed_flag = 0;

	curr_screen = 0;
	buffer = input_bufs[curr_screen];
	terminal_buf = ter_bufs[curr_screen];

	for(i = 0; i < MAX_TERMINALS; i++){
		input_indicies[i] = 0;
		terminal_indicies[i] = 0;
	}

	input_indicies[curr_screen] = input_indicies[curr_screen];
	terminal_indicies[curr_screen] = terminal_indicies[curr_screen];
}

/* handle_keyboard_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Reads in keyboard and map input to the correct typed character
 *		Side Effects: none
 */
void handle_keyboard_interrupt(){
	cli();
	temp_map_phys_vid();							// temporary switch vid memory mapping
	char kb_char = '\0';
	unsigned char keyboard_input = inb(KB_PORT);

	// conditions for different key presses
	switch(keyboard_input) {
		case TAB_PRESSED:
			kb_char = ' ';
			break;
		case BACKSPACE_PRESSED:
			handle_backspace();
			break;
		case ENTER_PRESSED:
			handle_enter();
			break;
		case CTRL_PRESSED:							//update control flag
			ctrl_flag = 1;
			break;
		case (CTRL_PRESSED + RELEASED_OFFSET):
			ctrl_flag = 0;
			break;
		case LSHIFT_PRESSED:						//update shift flag
			shift_flag = 1;
			break;
		case (LSHIFT_PRESSED + RELEASED_OFFSET):
			shift_flag = 0;
			break;
		case RSHIFT_PRESSED:
			shift_flag = 1;
			break;
		case (RSHIFT_PRESSED + RELEASED_OFFSET):
			shift_flag = 0;
			break;
		case ALT_PRESSED:							//update alt flag
			alt_flag = 1;
			break;
		case (ALT_PRESSED + RELEASED_OFFSET):
			alt_flag = 0;
			break;
		case CAPSLOCK_PRESSED:						//update capslock flag
			capslock_flag = capslock_flag ^ 1;
			break;
		case SPACE_PRESSED:
			kb_char = ' ';
			break;
		case F1_PRESSED:							//perform terminal switch when alt+function-key intercepted
			if (alt_flag) { terminal_switch(0); }
			break;
		case F2_PRESSED:
			if (alt_flag) { terminal_switch(1); }
			break;
		case F3_PRESSED:
			if (alt_flag) { terminal_switch(2); }
			break;
		case RIGHT_KEY_BYTE:
			switch(inb(KB_PORT)) {					//read secondbyte
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
			break;
		default:
			if((keyboard_input <= 0x35) && (keyboard_input > 0x01)){
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
				}
			}
			break;
	}


	if (kb_char != '\0') {
		if ((kb_char == 'L' || kb_char == 'l') && ctrl_flag == 1) {			//check ctrl+l or ctrl+L
			clear();
			update_cursor(0,0);												// move cursor to the top left
			int i;
			for (i = 0; i < input_indicies[curr_screen]; i++) {							//print keyboard buffer to keep/maintain state before ctrl+l
				putc(buffer[i]);
			}
		} else if ((kb_char == 'C' || kb_char == 'c') && ctrl_flag == 1) {	// check ctrl+c or ctrl+C
			//send_eoi(KB_IRQ);
			//squash_user_exception();		//.............................................
		} else if (input_indicies[curr_screen] < BUF_SIZE -1 && buffer_accessed_flag == 0) {	//if keyboard buffer not filled (127 chars, last char is '\n')
			putc(kb_char);												//prints char to screen and updates cursor
			// while (buffer_accessed_flag == 1);						//wait till terminal finishes clearing buffer
			buffer[input_indicies[curr_screen]] = kb_char;							//add char to keyboard buffer
			input_indicies[curr_screen]++;

			// update the temporary buffer used by terminal driver
			terminal_buf[terminal_indicies[curr_screen]] = kb_char;
			terminal_indicies[curr_screen]++;
		}
	}

	send_eoi(KB_IRQ);
	temp_map_switch_back();												// switch back mapping
	sti();
	return;
}

/* get_kb_buf
 *		Description: copies keyboard buffer into terminal driver's buffer
 * 		Inputs: buf = terminal buffer ptr
 				ter_num - terminal number
 * 		Return Value: index of last char in buffer
 */
int get_kb_buf(char* buf, int ter_num){
	int idx = terminal_indicies[ter_num];
	char * ter_buf = ter_bufs[ter_num];
	if(idx < 0 || idx >= BUF_SIZE){
		clear_terminal_buf(ter_num);
	}
	memcpy((void*)buf, (void*)ter_buf, idx);

	// return 0 when its empty
	return idx - 1 < 0 ? 0 : idx - 1;
}

/* clear_terminal_buf
 *		Description: clears the temporary buffer used by terminal driver
 * 		Inputs: ter_num - terminal number
 * 		Return Value: none
 */
void clear_terminal_buf(int ter_num){
	terminal_indicies[ter_num] = 0;						// reset the terminal index
	memset(ter_bufs[ter_num], '\0', BUF_SIZE);
}

/* clear_kb_buf
 *		Description: clears the keyboard buffer
 * 		Inputs: ter_num - terminal number
 * 		Return Value: none
 */
void clear_kb_buf(int ter_num) {
	input_indicies[ter_num] = 0;							// reset the terminal index
	memset(input_bufs[ter_num], '\0', BUF_SIZE);
}

/* handle_backspace
 *		Description: Updates kb buffer when backspace is pressed and change the display
 * 		Inputs: none
 * 		Return Value: none
 */
void handle_backspace() {
	// check to see if buffer is not empty
	if (input_indicies[curr_screen] > 0 && terminal_indicies[curr_screen] > 0) {
		input_indicies[curr_screen]--;
		buffer[input_indicies[curr_screen]] = '\0';						// backspace or null

		terminal_indicies[curr_screen]--;
		terminal_buf[terminal_indicies[curr_screen]] = '\0';				// backspace or null

		vid_backspace();
	}
}

/* handle_enter
 *		Description: Updates the buffers and change what is displayed when enter is pressed
 * 		Inputs: none
 * 		Return Value: none
 */
void handle_enter() {
	if (buffer_accessed_flag == 0) {				// ignore enters when handling enters
		buffer_accessed_flag = 1;					// enable flag

		buffer[input_indicies[curr_screen]] = '\n';				// which may cause chars to be added while input_indicies[curr_screen] = 0
		input_indicies[curr_screen]++;

		terminal_buf[terminal_indicies[curr_screen]] = '\n';		// which may cause chars to be added while input_indicies[curr_screen] = 0
		terminal_indicies[curr_screen]++;

		vid_enter();
		clear_kb_buf(curr_screen);
		if(!terminal_read_flag[curr_screen]){					// if keyboard interrupts not used by terminal read syscall,
			clear_terminal_buf(curr_screen);		// clear terminal driver's temporary buf as well
		}
		buffer_accessed_flag = 0;					// reset flag
	}
}

/* set_terminal_read_flag
 *		Description: when terminal read is in use, we set flag to 1
 * 		Inputs: flag - whether terminal read system call is running
 * 		Return Value: none
 */
void set_terminal_read_flag(int flag){
	terminal_read_flag[get_curr_scheduled()] = flag;
}

/* terminal_switch
 *		Description: ..................................................................................
 * 		Inputs: terminal_num - a terminal number 0-2
 * 		Return Value: none
 */
void terminal_switch(int ter_num) {

	//text_screen_map_update(get_curr_scheduled(), ter_num);//........update video mem
	//if(_get_pcb_arr()[_get_pid_tracker()[ter_num]]->vidmap_page_flag) {
	//	vidmap_update();
	//}

	int prev_screen = curr_screen;
	/* Do nothing if its the same terminal*/
	if (ter_num == curr_screen) { return; }

	//switch input buffer
	buffer = input_bufs[ter_num];
	terminal_buf = ter_bufs[ter_num];

	//saves current terminal's text screen and cursor position, then restores next terminal's
	curr_screen = ter_num;								//update current terminal number
	temp_map_phys_vid();

	vid_switch(prev_screen, ter_num);
	temp_map_switch_back();

}

/* get_curr_screen
 *		Description: returns which screen we are on
 * 		Inputs: none
 * 		Return Value: curr_screen - screen we are on right now
 */
int get_curr_screen(){
	return curr_screen;
}
