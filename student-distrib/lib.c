/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "keyboard.h"
#include "scheduler.h"

#define VIDEO		    0xB8000		 	// holds current video memory (text screen)
#define PAGE_TB_SIZE 	4096
#define NUM_COLS		80
#define NUM_ROWS		25
#define VIDEO_SIZE	    4096			// NUM_COLS*NUM_ROWS*2 <= 4096; num of bytes of 4kB page
#define MAX_TERMINALS   3
#define ATTRIB		    0x7

static int screen_x[MAX_TERMINALS];								// updated cursor positions for all terminals
static int screen_y[MAX_TERMINALS];

static char* video_mem = (char *)VIDEO;
static char* temp_vid_addr;										// temporary variable to store video address

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
	int32_t i;
	for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
		*(uint8_t *)(video_mem + (i << 1)) = ' ';
		*(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
	}
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *	   print 8 hexadecimal digits, zero-padded on the left.
 *	   For example, the hex number "E" would be printed as
 *	   "0000000E".
 *	   Note: This is slightly different than the libc specification
 *	   for the "#" modifier (this implementation doesn't add a "0x" at
 *	   the beginning), but I think it's more flexible this way.
 *	   Also note: %x is the only conversion specifier that can use
 *	   the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {
	/* Pointer to the format string */
	int8_t* buf = format;

	/* Stack pointer for the other parameters */
	int32_t* esp = (void *)&format;
	esp++;

	while (*buf != '\0') {
		switch (*buf) {
			case '%':
				{
					int32_t alternate = 0;
					buf++;

format_char_switch:
					/* Conversion specifiers */
					switch (*buf) {
						/* Print a literal '%' character */
						case '%':
							putc('%');
							break;

						/* Use alternate formatting */
						case '#':
							alternate = 1;
							buf++;
							/* Yes, I know gotos are bad.  This is the
							 * most elegant and general way to do this,
							 * IMHO. */
							goto format_char_switch;

						/* Print a number in hexadecimal form */
						case 'x':
							{
								int8_t conv_buf[64];
								if (alternate == 0) {
									itoa(*((uint32_t *)esp), conv_buf, 16);
									puts(conv_buf);
								} else {
									int32_t starting_index;
									int32_t i;
									itoa(*((uint32_t *)esp), &conv_buf[8], 16);
									i = starting_index = strlen(&conv_buf[8]);
									while(i < 8) {
										conv_buf[i] = '0';
										i++;
									}
									puts(&conv_buf[starting_index]);
								}
								esp++;
							}
							break;

						/* Print a number in unsigned int form */
						case 'u':
							{
								int8_t conv_buf[36];
								itoa(*((uint32_t *)esp), conv_buf, 10);
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a number in signed int form */
						case 'd':
							{
								int8_t conv_buf[36];
								int32_t value = *((int32_t *)esp);
								if(value < 0) {
									conv_buf[0] = '-';
									itoa(-value, &conv_buf[1], 10);
								} else {
									itoa(value, conv_buf, 10);
								}
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a single character */
						case 'c':
							putc((uint8_t) *((int32_t *)esp));
							esp++;
							break;

						/* Print a NULL-terminated string */
						case 's':
							puts(*((int8_t **)esp));
							esp++;
							break;

						default:
							break;
					}

				}
				break;

			default:
				putc(*buf);
				break;
		}
		buf++;
	}
	return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *	Function: Output a string to the console */
int32_t puts(int8_t* s) {
	register int32_t index = 0;
	while (s[index] != '\0') {
		putc(s[index]);
		index++;
	}
	return index;
}

/* void update_cursor();
 *	  Description: updates cursor position in text mode
 *	  Inputs: x = cursor's new x position
 *			  y = cursor's new y position
 *	  Outputs: none
 *	  Return Value: none
 */
void update_cursor(int x, int y) {

	screen_x[get_curr_screen()] = x;
	screen_y[get_curr_screen()] = y;

	uint16_t pos = y*NUM_COLS + x;

	*(uint8_t *)(video_mem + (pos << 1)) = ' ';														// 1st byte is ascii char
	*(uint8_t *)(video_mem + (pos << 1) + 1) = ATTRIB;												// 2nd byte is the background color

	outb(0x0F, 0x3D4);
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}


/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console 
 */
void putc(uint8_t c) {
	int flags = 0;
	// cli();
	cli_and_save(flags);		// avoid double faults
	/* If putc is called by a background process, write to the process (Not shown on screen)
	   If putc is called by keyboard or process on screen, write to the shown process (Shown on screen) */
	int i;
	int curr_sch = video_mapped_to_phys_screen() ? get_curr_screen(): get_curr_scheduled();
	
	if(c == '\n' || c == '\r') {
		screen_y[curr_sch]++;
		screen_x[curr_sch] = 0;
	} else {
		*(uint8_t *)(video_mem + ((NUM_COLS * screen_y[curr_sch] + screen_x[curr_sch]) << 1)) = c;
		*(uint8_t *)(video_mem + ((NUM_COLS * screen_y[curr_sch] + screen_x[curr_sch]) << 1) + 1) = ATTRIB;
		screen_x[curr_sch]++;
		screen_y[curr_sch] += (screen_x[curr_sch] / NUM_COLS);
		screen_x[curr_sch] %= NUM_COLS;
	}
	// vertical scrolling
	if (screen_y[curr_sch] >= NUM_ROWS) {
		unsigned space = 0x20;		// space ascii character
		memcpy((uint8_t *)(video_mem), (uint8_t *)(video_mem + NUM_COLS*2), (NUM_ROWS - 1) * NUM_COLS * 2); // shift video memory up, 2 bytes for each char
		for(i = 0; i < NUM_COLS; i++){
			*(uint8_t *)(video_mem + (NUM_ROWS - 1) * NUM_COLS * 2 + (i << 1)) = space;						// 1st byte is the ascii char
			*(uint8_t *)(video_mem + (NUM_ROWS - 1) * NUM_COLS * 2 + (i << 1) + 1) = ATTRIB;				// 2nd byte is the background color
		}
		// memset((uint8_t *)(video_mem + (NUM_ROWS - 1) * NUM_COLS * 2), space, NUM_COLS * 2);		// clears the last line, fill with spaces
		screen_y[curr_sch] = NUM_ROWS - 1; 																	// set to last row of screen
	}
																							 
	if (video_mapped_to_phys_screen()) {
		update_cursor(screen_x[curr_sch],screen_y[curr_sch]);
	} 
	// sti();
	restore_flags(flags);
}

/* void vid_backspace();
 *  Description: backspace effect in video memory
 *  Inputs: none
 *  Return Value: none
 *  Function: moves back to previous horizontal line or clears last non-empty space, updates cursor position
 */
void vid_backspace() {
	int curr_scr = get_curr_screen();
	if (screen_x[curr_scr] == 0) {				 //calculate new cursor position
		if (screen_y[curr_scr] > 0) {			 //deletes the '\n'
			screen_y[curr_scr]--;
			screen_x[curr_scr] = NUM_COLS-1;
		}
	} else {
		screen_x[curr_scr]--;
	}

	temp_map_phys_vid();
	*(uint8_t *)(video_mem + ((NUM_COLS * screen_y[curr_scr] + screen_x[curr_scr]) << 1)) = ' ';
	*(uint8_t *)(video_mem + ((NUM_COLS * screen_y[curr_scr] + screen_x[curr_scr]) << 1) + 1) = ATTRIB;
	update_cursor(screen_x[curr_scr], screen_y[curr_scr]);
	temp_map_switch_back();
}

/* void vid_enter();
 *  Description: enter effect in video memory
 *  Inputs: none
 *  Return Value: none
 *  Function: moves back to previous horizontal line or clears last non-empty space, updates cursor position
 */
void vid_enter() {
	putc('\n');
}

/* void vid_switch(int old_t_num, int new_t_num);
 *  Description: switches text screen and cursor position (helper for switching terminals)
 *  Inputs:
 *	  old_t_num - old/current terminal number
 *	  new_t_num - new terminal number
 *  Return Value: none
 *  Function: saves current terminal's text screen and cursor position, then restores new terminal's
 */
void vid_switch(int old_t_num, int new_t_num) {

	//.........................(get virtual addr of physical 0xB8000, since scheduling changes virtual 0xB8000 mapping).........................................................
	memcpy((void*) (VIDEO + (old_t_num + 1)*VIDEO_SIZE), (void*) VIDEO, VIDEO_SIZE);	// save - copy from current video mem to old terminal's background buffer
	memcpy((void*) (VIDEO), (void*) (VIDEO + (new_t_num + 1)*VIDEO_SIZE), VIDEO_SIZE);  // restore - copy from new terminal's background buffer to current video mem

	// save current terminal's cursor position
	// terminal_cursor_pos[old_t_num][0] = screen_x;
	// terminal_cursor_pos[old_t_num][1] = screen_y;

	// // restore new terminal's cursor position
	// screen_x = terminal_cursor_pos[new_t_num][0];
	// screen_y = terminal_cursor_pos[new_t_num][1];
	update_cursor(screen_x[new_t_num], screen_y[new_t_num]);
}

/*  text_screen_map_update
 * 		Inputs: curr_scheduled 	- current scheduling process
 				curr_screen 	- current process that's on screen
 * 		Return Value: none
 * 		Function: switches the video mapping of the processes, check if screen is the same as scheduled too
 * 		Side Effects: Flushes TLB after mapping
 *  
 */
void text_screen_map_update(int curr_scheduled, int curr_screen) {
	// page_table[VIDEO_MEM_IDX].addr = VIDEO_MEM_IDX + !(curr_screen == curr_scheduled) * (curr_scheduled + 1);// index of 4kB page of physical video memory
	video_mem = curr_screen == curr_scheduled ? (char*)VIDEO : (char*)(VIDEO + (curr_scheduled + 1) * PAGE_TB_SIZE);
	// flush_tlb();
}

/*  temp_map_phys_vid
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: temporary switches video memory mapping to physical vid memory
 * 		Side Effects: Flushes TLB after mapping
 */
void temp_map_phys_vid(){
	// switch mapping to video memory 
	if(video_mem != (char*)VIDEO){
		temp_vid_addr = video_mem;
		video_mem = (char*)VIDEO;
	}
	
	return;
}

/*  temp_map_switch_back
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: switches video memory mapping to previous mapping
 * 		Side Effects: Flushes TLB after mapping
 */
void temp_map_switch_back(){
	// switch mapping back
	if(temp_vid_addr != (char*)VIDEO){
		video_mem = temp_vid_addr;
		temp_vid_addr = (char*)VIDEO;
	}
	
	return;
}


/* save_cursor
 *  Description: switches text screen and cursor position (helper for switching terminals)
 *  Inputs:
 *	  terminal - which cursor to save
 *  Return Value: none
 *  Function: saves current terminal's text screen and cursor position
 */
void save_cursor(int terminal){
	// save current terminal's cursor position
	// terminal_cursor_pos[terminal][0] = screen_x;
	// terminal_cursor_pos[terminal][1] = screen_y;
	return;
}


/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *			int8_t* buf = allocated buffer to place string in
 *		  int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
	static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int8_t *newbuf = buf;
	int32_t i;
	uint32_t newval = value;

	/* Special case for zero */
	if (value == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return buf;
	}

	/* Go through the number one place value at a time, and add the
	 * correct digit to "newbuf".  We actually add characters to the
	 * ASCII string from lowest place value to highest, which is the
	 * opposite of how the number should be printed.  We'll reverse the
	 * characters later. */
	while (newval > 0) {
		i = newval % radix;
		*newbuf = lookup[i];
		newbuf++;
		newval /= radix;
	}

	/* Add a terminating NULL */
	*newbuf = '\0';

	/* Reverse the string and return */
	return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
	register int8_t tmp;
	register int32_t beg = 0;
	register int32_t end = strlen(s) - 1;

	while (beg < end) {
		tmp = s[end];
		s[end] = s[beg];
		s[beg] = tmp;
		beg++;
		end--;
	}
	return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
	register uint32_t len = 0;
	while (s[len] != '\0')
		len++;
	return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:	void* s = pointer to memory
 *		  int32_t c = value to set memory to
 *		 uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
	c &= 0xFF;
	asm volatile ("				 \n\
			.memset_top:			\n\
			testl   %%ecx, %%ecx	\n\
			jz	  .memset_done	\n\
			testl   $0x3, %%edi	 \n\
			jz	  .memset_aligned \n\
			movb	%%al, (%%edi)   \n\
			addl	$1, %%edi	   \n\
			subl	$1, %%ecx	   \n\
			jmp	 .memset_top	 \n\
			.memset_aligned:		\n\
			movw	%%ds, %%dx	  \n\
			movw	%%dx, %%es	  \n\
			movl	%%ecx, %%edx	\n\
			shrl	$2, %%ecx	   \n\
			andl	$0x3, %%edx	 \n\
			cld					 \n\
			rep	 stosl		   \n\
			.memset_bottom:		 \n\
			testl   %%edx, %%edx	\n\
			jz	  .memset_done	\n\
			movb	%%al, (%%edi)   \n\
			addl	$1, %%edi	   \n\
			subl	$1, %%edx	   \n\
			jmp	 .memset_bottom  \n\
			.memset_done:		   \n\
			"
			:
			: "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
	);
	return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:	void* s = pointer to memory
 *		  int32_t c = value to set memory to
 *		 uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
	asm volatile ("				 \n\
			movw	%%ds, %%dx	  \n\
			movw	%%dx, %%es	  \n\
			cld					 \n\
			rep	 stosw		   \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
	);
	return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:	void* s = pointer to memory
 *		  int32_t c = value to set memory to
 *		 uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
	asm volatile ("				 \n\
			movw	%%ds, %%dx	  \n\
			movw	%%dx, %%es	  \n\
			cld					 \n\
			rep	 stosl		   \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
	);
	return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:	  void* dest = destination of copy
 *		 const void* src = source of copy
 *			  uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
	asm volatile ("				 \n\
			.memcpy_top:			\n\
			testl   %%ecx, %%ecx	\n\
			jz	  .memcpy_done	\n\
			testl   $0x3, %%edi	 \n\
			jz	  .memcpy_aligned \n\
			movb	(%%esi), %%al   \n\
			movb	%%al, (%%edi)   \n\
			addl	$1, %%edi	   \n\
			addl	$1, %%esi	   \n\
			subl	$1, %%ecx	   \n\
			jmp	 .memcpy_top	 \n\
			.memcpy_aligned:		\n\
			movw	%%ds, %%dx	  \n\
			movw	%%dx, %%es	  \n\
			movl	%%ecx, %%edx	\n\
			shrl	$2, %%ecx	   \n\
			andl	$0x3, %%edx	 \n\
			cld					 \n\
			rep	 movsl		   \n\
			.memcpy_bottom:		 \n\
			testl   %%edx, %%edx	\n\
			jz	  .memcpy_done	\n\
			movb	(%%esi), %%al   \n\
			movb	%%al, (%%edi)   \n\
			addl	$1, %%edi	   \n\
			addl	$1, %%esi	   \n\
			subl	$1, %%edx	   \n\
			jmp	 .memcpy_bottom  \n\
			.memcpy_done:		   \n\
			"
			:
			: "S"(src), "D"(dest), "c"(n)
			: "eax", "edx", "memory", "cc"
	);
	return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:	  void* dest = destination of move
 *		 const void* src = source of move
 *			  uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
	asm volatile ("							 \n\
			movw	%%ds, %%dx				  \n\
			movw	%%dx, %%es				  \n\
			cld								 \n\
			cmp	 %%edi, %%esi				\n\
			jae	 .memmove_go				 \n\
			leal	-1(%%esi, %%ecx), %%esi	 \n\
			leal	-1(%%edi, %%ecx), %%edi	 \n\
			std								 \n\
			.memmove_go:						\n\
			rep	 movsb					   \n\
			"
			:
			: "D"(dest), "S"(src), "c"(n)
			: "edx", "memory", "cc"
	);
	return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *		 const int8_t* s2 = second string to compare
 *			   uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *			   in both strings form the same string.
 *			   A value greater than zero indicates that the first
 *			   character that does not match has a greater value
 *			   in str1 than in str2; And a value less than zero
 *			   indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
	int32_t i;
	for (i = 0; i < n; i++) {
		if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

			/* The s2[i] == '\0' is unnecessary because of the short-circuit
			 * semantics of 'if' expressions in C.  If the first expression
			 * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
			 * s2[i], then we only need to test either s1[i] or s2[i] for
			 * '\0', since we know they are equal. */
			return s1[i] - s2[i];
		}
	}
	return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:	  int8_t* dest = destination string of copy
 *		 const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
	int32_t i = 0;
	while (src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:	  int8_t* dest = destination string of copy
 *		 const int8_t* src = source string of copy
 *				uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
	int32_t i = 0;
	while (src[i] != '\0' && i < n) {
		dest[i] = src[i];
		i++;
	}
	while (i < n) {
		dest[i] = '\0';
		i++;
	}
	return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
	int32_t i;
	for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
		video_mem[i << 1]++;
	}
}

/* int is_current_screen(void)
 * Inputs: void
 * Return Value: 1 for video_mem = Video memory
 * Function: Determines if terminal screen is showing the processing pcb */
int video_mapped_to_phys_screen(){
	return (char*)(video_mem) == (char*)(VIDEO);
}

/*  get_video_mem()
 * 		Inputs: none
 * 		Return Value: none
 * 		Function: helper function to return the video_mem address
 * 		Side Effects: none
 */
char * get_video_mem() {
	return video_mem;
}
