#include "system_calls.h"
#include "filesystem.h"
#include "types.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "lib.h"

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
// REMINDER, SETUP STDIN, STDOUT FOPS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


/* execute
 *      Inputs: command - pointer to command
 *      Return Value: status to pass into halt
 *      Function: execute the executable
 *      Side Effects: may print stuff to screen
 */
int32_t execute(const uint8_t* command){
	// Step 1: Parse, remember to sanity check command
	int i = 0;
	char task_name[128];
	char task_args[128];

	// sanity checks
	if(command == NULL){
		return -1;
	}

	// find the location of the space character or null character
	while(true){
		// reached max, return -1 for failure
		if(i == 128){
			return -1;
		}

		// if the current character is null or space, then we found the complete task name
		if(command[i] == '\0' || command[i] == ' '){
			strncpy(task_name, command, i);				// copy over the task name over
			task_name[i] = '\0';						// makes sure that there's a null termination
			task_args[0] = '\0';						// set up the args string
			if(command[i] == '\0'){						// if there's a null termination, we're done
				break;
			}

			while(true){ 								// otherwise we need to get the argument as well
				if(i == 128){							// reached max, return -1 for failure
					return -1;
				}

				if(command[i] != ' '){					// once we see a non-space, we copy the rest of the string to args
					strcpy(task_args, &(command[i]));	// copy to args
					break;								// stop 
				}

				i++;									// increment if we are still on a space
			}
			break;										// break if we found a space or null
		}

		i++;											// increment until we find a space or null
	}

	// Step 2: Check if executable

	// Step 3: Setup paging
	if(exe_paging(PCB.pid) != 0){
		printf("Process ID invalid");
	}
	// Step 4: Load user program to user page

	// Step 5: context switch

}

}

/* exe_paging
 *      Inputs: pid - process id inside of PCB
 *      Return Value: 0 -- paging set up correclty
 *					  -1 -- pid value invalid
 *      Function: 128MB in virtual memory (user page) will map to physical memory for the tasks starting at 8MB
 *      Side Effects: Flushes the TLB after mapping
 */
int exe_paging(int pid){
	// check for a valid process id
	if (pid < 0){
		return -1;
	}
	// mapping user page to physical memory for tasks
	page_directory[USER_PAGE].addr 				= 	(2 + pid)  << SHFT_4MB_ADDR;					// address to the tasks starting at 8MB
	page_directory[USER_PAGE].accessed 			= 	0;												// not used, set to 0
	page_directory[USER_PAGE].dirty 			= 	0;												// not used, set to 0
	page_directory[USER_PAGE].global 			= 	1;												// we only set the page for kernel page
	page_directory[USER_PAGE].size 				= 	1;												// 1 for 4MB entry
	page_directory[USER_PAGE].available 		= 	0;												// not used, set to 0
	page_directory[USER_PAGE].cache_disable 	= 	1; 												// set to 1 for program code
	page_directory[USER_PAGE].write_through		= 	0; 												// we always want write back
	page_directory[USER_PAGE].user_supervisor 	= 	1; 												// user-level
	page_directory[USER_PAGE].read_write 		= 	1; 												// all pages are read write
	page_directory[USER_PAGE].present 			= 	1; 												// page available

	// flush the TLB
	asm(
		"movl 	%0, 			%%eax;"		// move page directory into eax
		"movl 	%%eax, 			%%cr3;"		// move page directory address into cr3

		"movl 	%%cr4, 			%%eax;"		// dump out cr4
		"orl 	$0x00000010, 	%%eax;"		// or the 4th bit of cr4
		"movl 	%%eax, 			%%cr4;"		// put eax contents back into cr4

		"movl 	%%cr0, 			%%eax;"		// dump out cr0
		"orl 	$0x80000000,	%%eax;"		// make the first and last bits 1
		"movl 	%%eax, 			%%cr0;"		// put it back into cr0

		:							// not outputs yet
		:"r"(page_directory) 		// input is page directory
		:"%eax" 					// clobbered register

		);

	return 0;
}
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
