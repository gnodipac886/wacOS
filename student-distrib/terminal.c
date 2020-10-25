#include "terminal.h"
#include "keyboard.h"
#include "lib.h"

/* terminal_open
 *      Description: initializes terminal
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
 *      Description: clears any terminal specific variable
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
 *      Description: Reads in input from the keyboard and store it in buf
 * 		Inputs: fd - file descriptor, should be 0 for reading
 *              buf - buffer that stores the keyboard inputs
 *              nbytes - number of bytes to read
 * 		Return Value: 0 - error, invalid pointer, or empty
 *                    number of bytes of characters
 * 		Function: Reads keyboard buffer and store it in buf. Checks for \n
 *                that signals the end of the keyboard buffer and writes \0 to
 *                the end of the buf.
 *		Side Effects: none
 */
int32_t terminal_read(int32_t fd, const void* buf, int32_t nbytes){
    // check for valid buf and correct fd for reading
    if((buf == NULL) | (fd != 0)){
        return 0;
    }
    clear_terminal_buf(buf);         // clear buf
    int32_t counter = 0;             // counter for iterating keyboard buffer
    char* kb_buf = get_kb_buf();     // pointer to the keyboard buffer
    // reads each char in keyboard buffer and checks for \n or reached nbytes
    while(counter != nbytes){
        if(counter == (nbytes - 1)){
            ((char*)buf)[nbytes - 1] = '\0';  // end of the buffer
        } else if((kb_buf[counter] == '\n') | (counter == 127)){
            ((char*)buf)[counter] = '\0';     // signals the end of the buffer
            return counter + 1;      // total bytes read
        } else{
            ((char*)buf)[counter] = kb_buf[counter];
        }
        counter++;
    }
    return nbytes;
}

/* terminal_write
 *      Description: Writes data in buf to the screen
 * 		Inputs: fd - file descriptor, should be 1 for writing
 *              buf - buffer that stores the data
 *              nbytes - number of bytes to write
 * 		Return Value: -1 - error, invalid pointer, or empty
 *                    number of bytes of characters written
 * 		Function: Writes nbytes of char to the screen from buf.
 *		Side Effects: none
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    // check for valid buf and correct fd for writing
    if((buf == NULL) | (fd != 1)){
        return -1;
    }
    int32_t i;                      // counter
    // loop through buf until all nbytes are written to the screen
    for(i = 0; i < nbytes; i++){
        if(i == NUM_COLS){
            printf("\n");           // newline reached max of the screen width
        }

        if(((char*)buf)[i] != '\0'){
            printf("%c", ((char*)buf)[i]);   // prints each char in buf, ignores null bytess
        }

    }
    return nbytes;
}

/* clear_terminal_buf
 *      Description: Clears the buffer
 * 		Inputs: buf - buffer that stores the terminal data
 * 		Return Value: none
 * 		Function: Clears the buf by putting in \0
 *		Side Effects: none
 */
void clear_terminal_buf(void* buf){
    int i = sizeof(buf)/sizeof(buf[0]); // size of the buf array

    memset(buf, '\0', i);               // clears buf, putting in null

    return;
}
