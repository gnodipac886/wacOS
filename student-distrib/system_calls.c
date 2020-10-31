#include "system_calls.h"
#include "filesystem.h"
#include "types.h"
#include "terminal.h"

/*
user:
ece391open(".")

ece391syscall.S
pushes args into registers
int $80

idt.c
skip syscallhandler and go straight to assembly linkage

assembly linkage
push arguments onto stack all 3 is fine

say open func:
switch statements for particular open (rtc, files, dir)

say rtc_open:
code



*/
// file descriptor array
file_descriptor_t fd_arr[MAX_FILES_OPEN];

f_ops_jmp_table_t rtc_file_ops		= 	{(void*)rtc_open, 		(void*)rtc_read, 		(void*)rtc_write, 		(void*)rtc_close};
f_ops_jmp_table_t dir_file_ops		= 	{(void*)directory_open, (void*)directory_read, 	(void*)directory_write, (void*)directory_close};
f_ops_jmp_table_t file_file_ops		= 	{(void*)file_open, 		(void*)file_read, 		(void*)file_write, 		(void*)file_close};
f_ops_jmp_table_t terminal_file_ops = 	{(void*)invalid_func, 	(void*)terminal_read, 	(void*)terminal_write, 	(void*)invalid_func};

/* open 
 *      Inputs: fname - name of file to open, should be "rtc"
 *      Return Value: file descriptor number
 *      Function: find the right array location have the file open, -1 on failure
 *      Side Effects: none     
 */
int32_t open(const uint8_t* fname){
	// set up variables for function
	int i;
	int fd = FIRST_FILE_IDX;
	dentry_t dentry;

	// check if the name is null
	if(fname == NULL){
		return -1;
	}

	// sanity check for fname
	for(i = 0; i < MAX_NAME_LEN + 1; i++){
		// found null, continue
		if(fname[i] == '\0'){
			break;
		}

		// didn't find it, return fail
		if(i == MAX_NAME_LEN && fname[i] != '\0'){
			return -1;
		}
	}

	// check if the file is found at all
	if(read_dentry_by_name(fname, &dentry) == -1){
		return -1;
	}

	// loop through the array to see which location in array is vacant
	while(fd_arr[fd].flags){
		fd++;

		// if the whole file array is full, return fail
		if(fd >= MAX_FILES_OPEN){
			return -1;
		}
	}


	switch (dentry.type) {
		case RTC_TYPE:
			fd_arr[fd].jmp_table = rtc_file_ops;
			break;

		case DIR_TYPE:
			fd_arr[fd].jmp_table = dir_file_ops;
			break;

		case FILE_TYPE:
			fd_arr[fd].jmp_table = file_file_ops;
			break;

		default:
			return -1;
	}

	fd_arr[fd].inode = dentry.type == FILE_TYPE ? dentry.inode : 0;
	fd_arr[fd].file_position = 0;										// 0 since we want the beginning of the file
	fd_arr[fd].flags = FILE_IN_USE;

	if((fd_arr[fd].jmp_table.f_ops_open)(fname) == -1){
		return -1;
	}

	return fd;
}

/* read
 *      Inputs: fd 		- file descriptor index value
 				buf 	- holds the name of the current file we are going to return, or inputs to print
 				nbytes 	- how many bytes to read
 *      Return Value: number of bytes read
 *      Function: dispatches to the other read for device, file, or directory
 *      Side Effects: none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
	// sanity checks
	if(fd >= MAX_FILES_OPEN || fd < STDIN || fd == STDOUT || fd_arr[fd].flags == FILE_NOT_USE ||buf == NULL){
		return -1;
	}

	if(fd == 0){
		return terminal_read(fd, buf, nbytes);
	}

	return (fd_arr[fd].jmp_table.f_ops_read)(fd, buf, nbytes);
}

/* write 
 *      Inputs: fd 		- file descriptor index value
 				buf 	- buffer that holds the data to write
 				nbytes 	- how many bytes to write
 *      Return Value: -1 regardless unless rtc in which case, 0
 *      Function: attempt to write to the file, but not implemented for now, 
 					write to rtc if file is of rtc type
 *      Side Effects: none     
 */
int32_t write(int32_t fd, void* buf, int32_t nbytes){
	// sanity checks
	if(fd >= MAX_FILES_OPEN || fd < STDIN || fd == STDIN || fd_arr[fd].flags == FILE_NOT_USE || buf == NULL){
		return -1;
	}

	if(fd == 1){
		return terminal_write(fd, buf, nbytes);
	}

	// call the corresponding write function
	return (fd_arr[fd].jmp_table.f_ops_write)(fd, buf, nbytes);
}

/* close 
 *      Inputs: fd - file descriptor index value
 *      Return Value: 0 on success, -1 upon failure
 *      Function: attempt to close the rtc in the array and reset it
 *      Side Effects: none     
 */
int32_t close(int32_t fd){
	// see if the file descriptor index is valid
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX){
		return -1;
	}

	// check if the file is used at all
	if(fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	// reset the file
	fd_arr[fd].inode = -1;
	fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file			
	fd_arr[fd].flags = FILE_NOT_USE;

	return (fd_arr[fd].jmp_table.f_ops_close)(fd);
}

/* invalid_func 
 *      Inputs: none
 *      Return Value: return -1
 *      Function: report failure
 *      Side Effects: none     
 */
int32_t invalid_func(){
	return -1;
}

/* _get_fd_arr 
 *      Inputs: none
 *      Return Value: return the file_descriptor_t
 *      Function: 
 *      Side Effects: none     
 */
file_descriptor_t* _get_fd_arr(){
	return fd_arr;
}
