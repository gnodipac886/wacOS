#include "system_calls.h"
#include "filesystem.h"
#include "types.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "lib.h"

/*
	1. when doing paging, do we have to disable the parent page?
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	2. integrate the fd_arr into pcb
	3. in halt, remember to switch tss->esp0 to parent kernel stack pointer
	4. cli needed in asm for context switch?
*/

int32_t curr_avail_pid = 0;
pcb_t* pcb_arr[MAX_TASKS];
int8_t pid_avail[MAX_TASKS] = {0, 0, 0, 0, 0, 0};					

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
	uint32_t i = 0;
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
			strncpy((int8_t*)task_name, (int8_t*)command, i);				// copy over the task name over
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
					strcpy((int8_t*)task_arg, (int8_t*)(&(command[i])));	// copy to args
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

	pcb = (pcb_t*)(KER_BOTTOM - (curr_avail_pid + 1) * KER_STACK_SIZE);

	// set up the file descriptor array and initialize to proper values
	for(i = 0; i < MAX_FILES_OPEN; i++){
		pcb->fd_arr[i].inode = -1;
		pcb->fd_arr[i].file_position = 0;
		pcb->fd_arr[i].flags = FILE_NOT_USE;
	}

	strcpy(pcb->arg, task_arg); 						// move the args into pcb
	pcb->pid = curr_avail_pid;	 						// set pid in the pcb
	// if current pid is 0, we are shell, so we ahve no parent
	pcb->parent_pid = pcb->pid == 0 ? 0 : _get_curr_pcb((int32_t*)&i)->pid;

	// set the pcb to global
	pcb_arr[curr_avail_pid] = pcb;

	// Step 2: Check if task_name file is a valid executable

	if(read_dentry_by_name((uint8_t*)task_name, &cur_dentry) == -1){			// Find file in file system and copy func info to cur_dentry
		return -1;
	}

	if (read_data(cur_dentry.inode, 1, (uint8_t*)ELF_check_buf, 3) != 3) {	// Error if cannot read starting three bytes into ELF_check_buf
		return -1;
	}

	if (strncmp((int8_t*)ELF_check_buf, (int8_t*)elf, 3) != 0) {						// compare starting three bytes of file with ELF
		return -1;
	}


	// Step 3: Setup paging
	if(exe_paging(pcb->pid, 1) != 0){					
		printf("Process ID invalid");
		return -1;
	}

	// Step 4: Load user program to user page
	read_data(cur_dentry.inode, 0, (uint8_t*)USR_PTR, _get_file_length_inode(cur_dentry.inode));		// read out the memory to the pointer    

	// Step 5: Kernel Stack TSS Update before context switch
	tss.esp0 = KER_BOTTOM - pcb->pid * KER_STACK_SIZE - sizeof(unsigned long);
	tss.ss0 = KERNEL_DS;

	// Step 6: context switch
		//halt (<--- has tss in function)
	// set up CRX registers

	asm volatile(
		"cli;"
		"pushl		%0;"						// push the SS which we use here the DS
		"pushl 		%2;"						// push address of the user stack
		"pushfl;"								// push the flags
		"pushl		%1;"						// push the code segment
		"pushl 		%3;"						// push the first line
		"movl 		%0, 		%%eax;"			// move DS to eax
		"movw 		%%ax, 		%%ds;"			// move DS into ds register
		"sti;"
		"iret;"									// perform context switch
		"halt_jmp_dest:;"
		"leave;"
		"ret;"
		:										// no outputs yet
		:"r"(USER_DS), "r"(USER_CS), "r"(USR_STACK), "r"(*(uint32_t*)(USR_PTR + FRST_INSTR)) // -1 to remain in stack, +1 to go to stack bottom
		:"eax"
		);
	return 0;
}

/* halt
 *      Inputs: status - executing status of the program
 *      Return Value: status variable
 *      Function: find the right array location have the file open, -1 on failure
 *      Side Effects: none
 */
int32_t halt(uint8_t status){
	int fd;
	pcb_t* pcb;

	pcb = _get_curr_pcb(&fd);						// get the current pcb

	// close all open files
	for(fd = 0; fd < MAX_FILES_OPEN; fd++){
		if(pcb->fd_arr[fd].flags == FILE_IN_USE){
			pcb->fd_arr[fd].inode = -1;
			pcb->fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file
			pcb->fd_arr[fd].flags = FILE_NOT_USE;
			(pcb->fd_arr[fd].jmp_table.f_ops_close)(fd);
		}
	}
	exe_paging(pcb->pid, 0);						// turn off paging for current user
	exe_paging(pcb->parent_pid, 1);					// revert back to parent paging

	pid_avail[pcb->pid] = 0; 						// reset the pid array
	curr_avail_pid = pcb->parent_pid; 				// set global pid to paretn's

	tss.esp0 = KER_BOTTOM - pcb->parent_pid * KER_STACK_SIZE - sizeof(unsigned long);
	tss.ss0 = KERNEL_DS;

	asm volatile(
		"xorl 		%%eax, 		%%eax;"				// clear eax
		"movzx		%%bl, 		%%eax;"				// move the argument status into eax for return
		"jmp 		halt_jmp_dest;"							// jump to the parent kernel stack
		:							// not outputs yet
		: 							// nothing here
		:"eax", "bl" 					// clobbered register
		);

	return -1;
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
	while((pcb_arr[curr_avail_pid])->fd_arr[fd].flags){
		fd++;

		// if the whole file array is full, return fail
		if(fd >= MAX_FILES_OPEN){
			return -1;
		}
	}


	switch (dentry.type) {
		case RTC_TYPE:
			(pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table = rtc_file_ops;
			break;

		case DIR_TYPE:
			(pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table = dir_file_ops;
			break;

		case FILE_TYPE:
			(pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table = file_file_ops;
			break;

		default:
			return -1;
	}

	(pcb_arr[curr_avail_pid])->fd_arr[fd].inode = dentry.type == FILE_TYPE ? dentry.inode : 0;
	(pcb_arr[curr_avail_pid])->fd_arr[fd].file_position = 0;										// 0 since we want the beginning of the file
	(pcb_arr[curr_avail_pid])->fd_arr[fd].flags = FILE_IN_USE;

	if(((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_open)(fname) == -1){
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
	if(fd >= MAX_FILES_OPEN || fd < STDIN || fd == STDOUT || buf == NULL){
		return -1;
	}

	if(fd == STDIN){
		return (stdin_ops.f_ops_read)(fd, buf, nbytes);
	}

	if((pcb_arr[curr_avail_pid])->fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	return ((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_read)(fd, buf, nbytes);
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
	if(fd >= MAX_FILES_OPEN || fd <= STDIN || buf == NULL){
		return -1;
	}

	if(fd == STDOUT){
		return (stdout_ops.f_ops_write)(fd, buf, nbytes);
	}

	if((pcb_arr[curr_avail_pid])->fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	// call the corresponding write function
	return ((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_write)(fd, buf, nbytes);
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
	if((pcb_arr[curr_avail_pid])->fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	// reset the file
	(pcb_arr[curr_avail_pid])->fd_arr[fd].inode = -1;
	(pcb_arr[curr_avail_pid])->fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file
	(pcb_arr[curr_avail_pid])->fd_arr[fd].flags = FILE_NOT_USE;

	return ((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_close)(fd);
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
	return (pcb_arr[curr_avail_pid])->fd_arr;
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
	return (pcb_t*)((uint32_t)ptr & PCB_MASK);
}
