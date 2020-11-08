#include "system_calls.h"
#include "filesystem.h"
#include "types.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "lib.h"

int32_t curr_avail_pid = 0;
pcb_t pcb_arr[MAX_TASKS];
int8_t pid_avail[MAX_TASKS] = {0, 0, 0, 0, 0, 0};					

// file descriptor array
file_descriptor_t fd_arr[MAX_FILES_OPEN];

f_ops_jmp_table_t rtc_file_ops		= 	{(void*)rtc_open, 		(void*)rtc_read, 		(void*)rtc_write, 		(void*)rtc_close};
f_ops_jmp_table_t dir_file_ops		= 	{(void*)directory_open, (void*)directory_read, 	(void*)directory_write, (void*)directory_close};
f_ops_jmp_table_t file_file_ops		= 	{(void*)file_open, 		(void*)file_read, 		(void*)file_write, 		(void*)file_close};
f_ops_jmp_table_t stdin_ops			= 	{(void*)invalid_func, 	(void*)terminal_read, 	(void*)invalid_func, 	(void*)invalid_func};
f_ops_jmp_table_t stdout_ops		= 	{(void*)invalid_func, 	(void*)invalid_func, 	(void*)terminal_write, 	(void*)invalid_func};

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
	char task_arg[128];
	pcb_t* pcb;
	char elf[] = "ELF";
	char ELF_check_buf[3];
	dentry_t cur_dentry;

	// sanity checks
	if(command == NULL){
		return -1;
	}

	// find the location of the space character or null character
	while(1){
		// reached max, return -1 for failure
		if(i == 128){
			return -1;
		}

		// if the current character is null or space, then we found the complete task name
		if(command[i] == '\0' || command[i] == ' '){
			strncpy(task_name, command, i);				// copy over the task name over
			task_name[i] = '\0';						// makes sure that there's a null termination
			task_arg[0] = '\0';						// set up the args string
			if(command[i] == '\0'){						// if there's a null termination, we're done
				break;
			}

			while(1){ 								// otherwise we need to get the argument as well
				if(i == 128){							// reached max, return -1 for failure
					return -1;
				}

				if(command[i] != ' '){					// once we see a non-space, we copy the rest of the string to args
					strcpy(task_arg, &(command[i]));	// copy to args
					break;								// stop
				}

				i++;									// increment if we are still on a space
			}
			break;										// break if we found a space or null
		}

		i++;											// increment until we find a space or null
	}

	for (curr_avail_pid = 0; curr_avail_pid < MAX_TASKS; curr_avail_pid++) {
		if (pid_avail[curr_avail_pid] == 0) {
			pid_avail[curr_avail_pid] = 1;
			break;
		}
	}

	pcb = KER_BOTTOM - (curr_avail_pid + 1) * KER_STACK_SIZE;

	strcpy(pcb->arg, task_arg); 						// move the args into pcb
	pcb->pid = curr_avail_pid;	 						// set pid in the pcb
	// if current pid is 0, we are shell, so we ahve no parent
	pcb->parent_pid = pcb->pid == 0 ? 0 : _get_curr_pcb(&i)->pid;

	// Step 2: Check if task_name file is a valid executable

	if(read_dentry_by_name(task_name, &cur_dentry) == -1){			// Find file in file system and copy func info to cur_dentry
		return -1;
	}

	if (read_data(cur_dentry.inode, 0, ELF_check_buf, 3) != 3) {	// Error if cannot read starting three bytes into ELF_check_buf
		return -1;
	}

	if (strncmp(ELF_check_buf, elf, 3) != 0) {						// compare starting three bytes of file with ELF
		return -1;
	}


	// Step 3: Setup paging
	if(exe_paging(pcb->pid) != 0){					
		printf("Process ID invalid");
	}

	// Step 4: Load user program to user page
	read_data(cur_dentry.inode, 0, USR_PTR, _get_file_length_inode(cur_dentry.inode));		// read out the memory to the pointer    

	// Step 5: Kernel Stack TSS Update before context switch
	tss.esp0 = KER_BOTTOM - pcb->pid * KER_STACK_SIZE - sizeof(unsigned long);
	tss.ss0 = KERNEL_DS;

	// Step 6: context switch
		//halt (<--- has tss in function)
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
	if(fd >= MAX_FILES_OPEN || fd < STDIN || fd == STDOUT || fd_arr[fd].flags == FILE_NOT_USE || buf == NULL){
		return -1;
	}

	if(fd == 0){
		return (stdin_ops.f_ops_read)(fd, buf, nbytes);
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
		return (stdout_ops.f_ops_write)(fd, buf, nbytes);
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

/* _get_curr_pcb
 *      Inputs: ptr - pointer to anything on the kernel stack
 *      Return Value: pcb* pointer to pcb at the top of the stack
 *      Function:
 *      Side Effects: none
 */
pcb_t* _get_curr_pcb(int32_t* ptr){
	// check if its in the kernel range at all
	if((uint32_t)ptr >= KER_BOTTOM || (uint32_t)ptr < KER_TOP){
		return NULL;
	}

	// bitwise and with the mask and return the pointer
	return (pcb_t*)(ptr & PCB_MASK);
}