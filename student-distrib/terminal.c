#include "terminal.h"
#include "keyboard.h"
#include "lib.h"

/* terminal_open
 *	  Description: initializes terminal
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Initializes anything that terminal needs or do nothing
 *		Side Effects: none
 */
int32_t terminal_open(){
	//  initializes terminal stuff (or nothing)
	return 0;
}

/* terminal_close
 *	  Description: clears any terminal specific variable
 * 		Inputs: none
 * 		Return Value: 0
 * 		Function: Clears anything terminal initialized or do nothing
 *		Side Effects: none
 */
int32_t terminal_close(){
	// clears any terminal specific variables or do nothing
	return 0;
}

/* terminal_read
 *	  Description: Reads in input from the keyboard and store it in buf
 * 		Inputs: fd - file descriptor, should be 0 for reading
 *			  buf - user space buffer that stores the keyboard inputs
 *			  nbytes - number of bytes to read
 * 		Return Value: 0 - error, invalid pointer, or empty
 *					number of bytes of characters
 * 		Function: Reads keyboard buffer and store it in buf. Checks for \n
 *				that signals the end of the keyboard buffer and writes \0 to
 *				the end of the buf.
 *		Side Effects: none
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
	// check for valid buf and correct fd for reading
	if((buf == NULL) | (fd != 0)){
		return 0;
	}

	memset(buf, '\0', BUF_SZ);                    // clear buf
	char kb_buf[BUF_SZ];
	int kb_buf_idx;

    // keeps checking until keyboard enters \n or filled buffer
	while(1){
		memset(kb_buf, '\0', BUF_SZ);
		kb_buf_idx = get_kb_buf(kb_buf);	     // idx of last added char in keyboard buffer (copy keyboard buffer)

		if(kb_buf[kb_buf_idx] == '\n') {
			clear_terminal_buf();			     //clear keyboard handler buffer
			break;
		}
	}
	memcpy(buf, (void*)kb_buf, kb_buf_idx + 1);	 //kb_buf_idx + 1 = number of bytes written to user space buffer
	return kb_buf_idx + 1;                       // number of bytes read
}

/* terminal_write
 *	  Description: Writes data in buf to the screen
 * 		Inputs: fd - file descriptor, should be 1 for writing
 *			  buf - buffer that stores the data
 *			  nbytes - number of bytes to write
 * 		Return Value: -1 - error, invalid pointer, or empty
 *					number of bytes of characters written
 * 		Function: Writes nbytes of char to the screen from buf.
 *		Side Effects: none
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
	// check for valid buf and correct fd for writing
	if((buf == NULL) | (fd != 1)){
		return -1;
	}
	int32_t i;					  // counter
	int32_t bytes_written = 0;
	// loop through buf until all nbytes are written to the screen
	for(i = 0; i < nbytes; i++){
        // check for null terminating char if not print it
		if(((char*)buf)[i] != '\0'){
			putc(((char*)buf)[i]);
			bytes_written++;
		}
	}
	return bytes_written;
}
