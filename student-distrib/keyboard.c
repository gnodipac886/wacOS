#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
// 0x02 - 0x0D from 1 to =
char kb_sc_row0_nums[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '='};
// 0x10 - 0x1B from q to ]
char kb_sc_row1_let[] = {'q', 'w', 'e', 'r', 't', 'y', 'u','i', 'o', 'p', '[', ']'};
// 0x1E - 0x29 from a to (backtick)
char kb_sc_row2_let[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`'};
// 0x2B - 0x35 from \ (row 2) z(row 3) to /
char kb_sc_row3_let[] = {'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'};

/* __keyboard_init__
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: initialization of the keyboard for interrupt
 *		Side Effects: none
 */
void __keyboard_init__(){
	enable_irq(KB_IRQ);
}

/* handle_keyboard_interrupt
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: Reads in keyboard and map input to the correct typed character
 *		Side Effects: none
 */
void handle_keyboard_interrupt(){
	cli();

	char kb_char;
	char keyboard_input = inb(KB_PORT);

	// space pressed = 0x39
	if(keyboard_input == 0x39){
		kb_char = ' ';
	} else if(keyboard_input <= 0x35 & keyboard_input > 0x01){
		// between 0x35 = /, 0x01 = esc on keyboard

				// between 0x02 = 1 and 0x0D = "="
				if(keyboard_input <= 0x0D && keyboard_input >= 0x02){
					kb_char = kb_sc_row0_nums[keyboard_input - 2]; // -2 for the offset mapping in the array
				} else if(keyboard_input <= 0x1B && keyboard_input >= 0x10){
					// between 0x10 = q and 0x1B = ]
					kb_char = kb_sc_row1_let[keyboard_input - 0x10]; // 0x10 for the offset mapping in the array
				} else if(keyboard_input <= 0x29 && keyboard_input >= 0x1E){
					// between 0x1E = a and 0x29 =  `
					kb_char = kb_sc_row2_let[keyboard_input - 0x1E]; // 0x1E for the offset mapping in the array
				} else if(keyboard_input <= 0x35 && keyboard_input >= 0x2B){
					// between 0x2B = \ and 0x35 = /
					kb_char = kb_sc_row3_let[keyboard_input - 0x2B]; // 0x2C for the offset mapping in the array
				} else{
					send_eoi(KB_IRQ);
					sti();
					return;
				}
	} else{
		send_eoi(KB_IRQ);
		sti();
		return;
		}
	printf("%c", kb_char);
	send_eoi(KB_IRQ);
	sti();
	return;
}
