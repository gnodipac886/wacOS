#include "system_calls.h"
#include "filesystem.h"
#include "types.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "lib.h"

// global variables
int32_t curr_avail_pid = 0;
pcb_t* pcb_arr[MAX_TASKS];
int8_t pid_avail[MAX_TASKS] = {0, 0, 0, 0, 0, 0};
extern uint32_t is_exception;

// fops table for each type of file possible
static f_ops_jmp_table_t rtc_file_ops		= 	{(void*)rtc_open, 		(void*)rtc_read, 		(void*)rtc_write, 		(void*)rtc_close};
static f_ops_jmp_table_t dir_file_ops		= 	{(void*)directory_open, (void*)directory_read, 	(void*)directory_write, (void*)directory_close};
static f_ops_jmp_table_t file_file_ops		= 	{(void*)file_open, 		(void*)file_read, 		(void*)file_write, 		(void*)file_close};
static f_ops_jmp_table_t stdin_ops			= 	{(void*)invalid_func, 	(void*)terminal_read, 	(void*)invalid_func, 	(void*)invalid_func};
static f_ops_jmp_table_t stdout_ops			= 	{(void*)invalid_func, 	(void*)invalid_func, 	(void*)terminal_write, 	(void*)invalid_func};

/* execute
 *      Inputs: command - pointer to start of command string
 *      Return Value: status to pass into halt; -1 on failure
 *      Function: execute the executable - creates PCB for task/process, sets up paging for new task, 
 * 										   loads user program to user page, context switches to user program
 *      Side Effects: may print stuff to screen
 */
int32_t execute(const uint8_t* command){
	/***************************************
	**************Step 1: Parse*************
	***************************************/

	// initialize variables
	uint32_t i = 0;
	uint32_t parent_k_esp;
	uint32_t parent_k_ebp;
	char task_name[KB_BUF_SIZE];
	char task_arg[KB_BUF_SIZE];
	pcb_t* pcb;
	char elf[] = {(char)0x7f,'E','L','F'};									// magic number at front of executable files
	char ELF_check_buf[4];
	dentry_t cur_dentry;

	// sanity checks
	if(command == NULL){
		return -1;
	}

	while(1){ 																// find the location of the space character or null character
		if(i == KB_BUF_SIZE){												// reached max, return -1 for failure
			return -1;
		}

		if(command[i] == '\0' || command[i] == ' '){ 						// if the current character is null or space, then we found the complete task name
			strncpy((int8_t*)task_name, (int8_t*)command, i);				// copy over the task name over
			task_name[i] = '\0';											// makes sure that there's a null termination
			task_arg[0] = '\0';												// set up the args string
			if(command[i] == '\0'){											// if there's a null termination, we're done
				break;
			}

			while(1){ 														// otherwise we need to get the argument as well
				if(i == KB_BUF_SIZE){										// reached max, return -1 for failure
					return -1;
				}

				if(command[i] != ' '){										// once we see a non-space, we copy the rest of the string to args
					strcpy((int8_t*)task_arg, (int8_t*)(&(command[i])));	// copy to args
					break;													// stop
				}

				i++;														// increment if we are still on a space
			}
			break;															// break if we found a space or null
		}

		i++;																// increment until we find a space or null
	}

	// try to allocate a task
	for (curr_avail_pid = 0; curr_avail_pid < MAX_TASKS; curr_avail_pid++) {
		if (pid_avail[curr_avail_pid] == 0) {
			pid_avail[curr_avail_pid] = 1;
			break;
		}
		if(curr_avail_pid == MAX_TASKS - 1){								// if we haven't found a open space
			return 255;														// 255 for just failure in general, not -1 (command dne)
		}
	}

	/***************************************
	******Step 2: Create PCB structure******
	***************************************/

	pcb = (pcb_t*)(KER_BOTTOM - (curr_avail_pid + 1) * KER_STACK_SIZE);		// find the right address to store pcb

	for(i = 0; i < MAX_FILES_OPEN; i++){									// set up the file descriptor array and initialize to proper values
		pcb->fd_arr[i].inode = -1;
		pcb->fd_arr[i].file_position = 0;
		pcb->fd_arr[i].flags = FILE_NOT_USE;
	}

	strcpy(pcb->arg, task_arg); 											// move the args into pcb
	//pcb->vidmap_page_flag = 0;												// no paging set up for this pcb yet
	// ............................check if we need to return to previous state of vidmap_page_flag of parent process
	pcb->pid = curr_avail_pid;	 											// set pid in the pcb
	pcb->parent_pid = pcb->pid == 0 ? 0 : _get_curr_pcb((int32_t*)&i)->pid; // if current pid is 0, we are shell, so we ahve no parent

	// store parent kernel stack info - esp and ebp
	asm volatile(
		"movl	%%esp, 	%0;"
		"movl	%%ebp, 	%1;"
		:"=g" (parent_k_esp), "=g" (parent_k_ebp)							// outputs - temp vars to be used to set pcb values
	);

	pcb->parent_kernel_esp = parent_k_esp;									// set parent esp and ebp into pcb
	pcb->parent_kernel_ebp = parent_k_ebp;

	pcb_arr[curr_avail_pid] = pcb;											// set the pcb to global

	/***************************************
	*****Step 3: Check valid executable*****
	***************************************/

	if(read_dentry_by_name((uint8_t*)task_name, &cur_dentry) == -1){		// Find file in file system and copy func info to cur_dentry
		pid_avail[curr_avail_pid] = 0; 										// remove task
		return -1;
	}

	if (read_data(cur_dentry.inode, 0, (uint8_t*)ELF_check_buf, 4) != 4) {	// Error if cannot read starting three bytes into ELF_check_buf, 4 for elf length
		pid_avail[curr_avail_pid] = 0;										// remove task
		return -1;
	}

	if (strncmp((int8_t*)ELF_check_buf, (int8_t*)elf, 4) != 0) {			// compare starting three bytes of file with ELF, 4 for elf length
		pid_avail[curr_avail_pid] = 0;										// remove task
		return -1;
	}

	/***************************************
	**********Step 4: Setup paging**********
	***************************************/
	if(exe_paging(pcb->pid, 1) != 0){										// try to do paging				
		printf("Process ID invalid");
		pid_avail[curr_avail_pid] = 0;
		return -1;
	}

	/***************************************
	*Step 5: Load user program to user page*
	***************************************/
	read_data(cur_dentry.inode, 0, (uint8_t*)USR_PTR, _get_file_length_inode(cur_dentry.inode));		// read out the memory to the pointer    

	/*************************************
	**********Step 6: Update TSS**********
	*************************************/
	tss.esp0 = KER_BOTTOM - pcb->pid * KER_STACK_SIZE - sizeof(unsigned long);
	tss.ss0 = KERNEL_DS;

	/***************************************
	********Step 7: context switch**********
	***************************************/
	asm volatile(
		"cli;"
		"pushl		%0;"													// push the SS which we use here the DS
		"pushl 		%2;"													// push address of the user stack
		"pushfl;"															// push the flags
		"orl 		$0x200, 	(%%esp);"									// or the IF bit so when we iret, it sti's
		"pushl		%1;"													// push the code segment
		"pushl 		%3;"													// push the first line
		"movl 		%0, 		%%eax;"										// move DS to eax
		"movw 		%%ax, 		%%ds;"										// move DS into ds register
		"sti;"
		"iret;"																// perform context switch
		"halt_jmp_dest:;"
		"leave;"
		"ret;"
		:																	// no outputs yet
		:"r"(USER_DS), "r"(USER_CS), "r"(USR_STACK), "r"(*(uint32_t*)(USR_PTR + FRST_INSTR)) // -1 to remain in stack, +1 to go to stack bottom
		:"eax"
		);
	return 0;
}

/* halt
 *      Inputs: status - executing status of the program
 *      Return Value: status variable; -1 on failure
 *      Function: - close associated files, turn off current paging, revert back to parent paging, 
 * 				  return to use old PCB, return to parent kernel stack
 * 				  - reboots shell if necessary
 *      Side Effects: none
 */
int32_t halt(uint8_t status){
	int fd;
	uint32_t parent_k_esp;
	uint32_t parent_k_ebp;
	pcb_t* pcb = _get_curr_pcb(&fd);										// get the current pcb
	pcb_t* par_pcb = pcb_arr[pcb->parent_pid]; 								// get the parent pcb array

	for(fd = 0; fd < MAX_FILES_OPEN; fd++){									// close all open files
		if(pcb->fd_arr[fd].flags == FILE_IN_USE){
			pcb->fd_arr[fd].inode = -1;
			pcb->fd_arr[fd].file_position = 0;								// 0 since we want the beginning of the file
			pcb->fd_arr[fd].flags = FILE_NOT_USE;
			(pcb->fd_arr[fd].jmp_table.f_ops_close)(fd);
		}
	}

	if (par_pcb->vidmap_page_flag != 1 && pcb->vidmap_page_flag){
		// deallocate 4kB page
		pcb->vidmap_page_flag = 0;											// change the present bit in the PCB
		vidmap_pte_setup(NULL, 0);											// revert the paging setup
	}											

	exe_paging(pcb->pid, 0);												// turn off paging for current user
	exe_paging(pcb->parent_pid, 1);											// revert back to parent paging

	parent_k_esp = pcb->parent_kernel_esp;									// restore esp and ebp of parent 
	parent_k_ebp = pcb->parent_kernel_ebp;

	pid_avail[pcb->pid] = 0; 												// reset the pid array
	curr_avail_pid = pcb->parent_pid; 										// set global pid to parent's

	// reset the tss
	tss.esp0 = KER_BOTTOM - pcb->parent_pid * KER_STACK_SIZE - sizeof(unsigned long);
	tss.ss0 = KERNEL_DS;

	clear_terminal_buf();													// clear keyboard buffer to prevent deleting the shell prompt
	clear_kb_buf();

	if(pcb->pid == 0 && pcb->parent_pid == 0){								// base shell case
		execute((uint8_t*)"shell");
	}
	asm volatile(
		"movl		%0,			%%esp;"										// restore esp for parent kernel stack
		"movl		%1,			%%ebp;"										// restore ebp for parent kernel stack			
		"xorl 		%%eax, 		%%eax;"										// clear eax
		"movzx		%%bl, 		%%eax;"										// move the argument status into eax for return
		"cmpl 		$256, 		%2;"										// see if we need to load 256 for exceptions
		"jne 		halt_jmp_dest;"											// jump to the parent kernel stack
		"movl 		%2, 		%%eax;"										// load 256 into eax for returning
		"jmp 		halt_jmp_dest;"											// jump to the parent kernel stack
		:																	// not outputs yet
		:"r" (parent_k_esp), "r" (parent_k_ebp), "r"(is_exception)			// esp and ebp values to restore for parent kernel stack
		:"eax", "bl" 														// clobbered register
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

	if(fname == NULL){														// check if the name is null
		return -1;
	}

	// sanity check for fname
	for(i = 0; i < MAX_NAME_LEN + 1; i++){
		if(fname[i] == '\0'){												// found null, continue
			break;
		}

		if(i == MAX_NAME_LEN && fname[i] != '\0'){							// didn't find it, return fail
			return -1;
		}
	}

	if(read_dentry_by_name(fname, &dentry) == -1){							// check if the file is found at all
		return -1;
	}

	while((pcb_arr[curr_avail_pid])->fd_arr[fd].flags){						// loop through the array to see which location in array is vacant
		fd++;

		if(fd >= MAX_FILES_OPEN){											// if the whole file array is full, return fail
			return -1;
		}
	}


	switch (dentry.type) {													// depending on the fine type, we set the fops table
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

	(pcb_arr[curr_avail_pid])->fd_arr[fd].inode = dentry.type == FILE_TYPE ? dentry.inode : 0;		// set the fd arr to support the file type
	(pcb_arr[curr_avail_pid])->fd_arr[fd].file_position = 0;										// 0 since we want the beginning of the file
	(pcb_arr[curr_avail_pid])->fd_arr[fd].flags = FILE_IN_USE;

	if(((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_open)(fname) == -1){	 				// call the filetype specific open function
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

	memset(buf, '\0', nbytes);																		// make sure to clear the buffer before reading

	if(fd == STDIN){																				// if fd is 0, then we do terminal read
		return (stdin_ops.f_ops_read)(fd, buf, nbytes);
	}

	if((pcb_arr[curr_avail_pid])->fd_arr[fd].flags == FILE_NOT_USE){								// if file is not in use, we return -1
		return -1;
	}

	return ((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_read)(fd, buf, nbytes); 			// we call file type specific read
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

	if(fd == STDOUT){																				// if its 1, then we call terminal write
		return (stdout_ops.f_ops_write)(fd, buf, nbytes);
	}

	if((pcb_arr[curr_avail_pid])->fd_arr[fd].flags == FILE_NOT_USE){								// if file not in use, then we return -1
		return -1;
	}

	return ((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_write)(fd, buf, nbytes);			// call the corresponding write function
}

/* close
 *      Inputs: fd - file descriptor index value
 *      Return Value: 0 on success, -1 upon failure
 *      Function: attempt to close the rtc in the array and reset it
 *      Side Effects: none
 */
int32_t close(int32_t fd){
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX){												// see if the file descriptor index is valid
		return -1;
	}

	if((pcb_arr[curr_avail_pid])->fd_arr[fd].flags == FILE_NOT_USE){ 								// check if the file is used at all
		return -1;
	}

	// reset the file
	(pcb_arr[curr_avail_pid])->fd_arr[fd].inode = -1;
	(pcb_arr[curr_avail_pid])->fd_arr[fd].file_position = 0;										// 0 since we want the beginning of the file
	(pcb_arr[curr_avail_pid])->fd_arr[fd].flags = FILE_NOT_USE;

	return ((pcb_arr[curr_avail_pid])->fd_arr[fd].jmp_table.f_ops_close)(fd);
}

/* getargs
 *      Inputs: buf 	- buffer to copy the args into
 				nbytes 	- how many bytes of the argument to copy
 *      Return Value: 0 on success, -1 upon failure
 *      Function: gives the user the args
 *      Side Effects: none
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
	int i = 0;																						// create random variable on stack
	pcb_t* pcb = _get_curr_pcb(&i); 																// get pcb from kernel stack

	// sanity checks
	if(buf == NULL || pcb->arg[i] == '\0' || strlen(pcb->arg) >= nbytes){
		return -1;
	}

	strcpy((int8_t*)buf, (int8_t*)(pcb->arg));														// copy the argument into the buffer
	return 0;
}

/* vidmap
 *      Inputs: screen_start - addr to copy the new 4kB page's start addr into
 *      Return Value: 0 on success, -1 upon failure
 *      Function: copies the (virtual) ptr to start of new 4kB page (pointing to physical video memory page)
 * 				  into screen_start double ptr passed by user program
 *      Side Effects: Allows user program to write to vid mem directly
 */
int32_t vidmap(uint8_t ** screen_start){
	int i = 0; 											// set up variable for getting pcb

	if (screen_start == NULL || screen_start > (uint8_t**)(USR_BOTTOM - sizeof(uint8_t*)) || screen_start < (uint8_t**)USR_PTR) {					// check if screen_start argument is valid
		return -1;
	}
	
	if (vidmap_pte_setup(screen_start, 1) == -1) { 		// try to set up the paging for vidmap
		return -1;
	}

	pcb_t* pcb = _get_curr_pcb(&i);
	pcb->vidmap_page_flag = 1;
	
	return 0;
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
	return (pcb_arr[curr_avail_pid])->fd_arr; 														// return the current fd array
}

/* _get_curr_pcb
 *      Inputs: ptr - pointer to anything on the kernel stack
 *      Return Value: pcb* pointer to pcb at the top of the stack
 *      Function:
 *      Side Effects: none
 */
pcb_t* _get_curr_pcb(int32_t* ptr) {
	if((uint32_t)ptr >= KER_BOTTOM || (uint32_t)ptr < KER_TOP) { 									// check if its in the kernel range at all
		return NULL;
	}

	return (pcb_t*)((uint32_t)ptr & PCB_MASK); 														// bitwise and with the mask and return the pointer
}
