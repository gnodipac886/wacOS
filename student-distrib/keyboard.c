#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

void __keyboard_init__(){
	enable_irq(KB_IRQ);
}

void handle_keyboard_interrupt(){
	cli();
	clear();
	printf("Keyboard\n");

	send_eoi(KB_IRQ);
	sti();
}
