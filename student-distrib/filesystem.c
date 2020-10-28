#include "filesystem.h"
#include "lib.h"
#include "rtc.h"

// global variables
static void * filesystem;
boot_block_t* boot_block;
inode_t* inodes;
data_block_t* data_ptr;

// file descriptor array
file_descriptor_t fd_arr[MAX_FILES_OPEN];

// 4 for number of functions
uint32_t rtc_file_ops[4] = {(uint32_t)rtc_open, (uint32_t)rtc_read, (uint32_t)rtc_write, (uint32_t)rtc_close};
uint32_t dir_file_ops[4] = {(uint32_t)directory_open, (uint32_t)directory_read, (uint32_t)directory_write, (uint32_t)directory_close};
uint32_t file_file_ops[4] = {(uint32_t)file_open, (uint32_t)file_read, (uint32_t)file_write, (uint32_t)file_close};

/* __init_filesystem__ 
 *      Inputs: filesystem_ptr - pointer to the beginning of the file system 
 *      Return Value: None
 *      Function: Initialize the file system
 *      Side Effects: none     
 */
void __init_filesystem__(void * filesystem_ptr){
	// check to see if the pointer passed in is valid or not
	if(filesystem_ptr == NULL){
		return;
	}

	// set up variables and pointers for each data structure
	int i;
	filesystem 	= filesystem_ptr;
	boot_block 	= (boot_block_t*)filesystem;
	inodes 		= (inode_t*)filesystem;
	data_ptr 	= (data_block_t*)filesystem;

	// set up the file descriptor array and initialize to proper values
	for(i = 0; i < MAX_FILES_OPEN; i++){
		fd_arr[i].jmp_table = NULL;
		fd_arr[i].inode = -1;
		fd_arr[i].file_position = 0;
		fd_arr[i].flags = FILE_NOT_USE;
	}
}

/* rtc_open 
 *      Inputs: fname - name of file to open, should be "rtc"
 *      Return Value: file descriptor number
 *      Function: find the right array location have the file open, -1 on failure
 *      Side Effects: none     
 */
int32_t rtc_open(const uint8_t* fname){
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

	// set the jump table
	fd_arr[fd].jmp_table = rtc_file_ops;

	// if we are opening the rtc, we set it up too
	_rtc_open();

	fd_arr[fd].inode = 0;
	fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file
	fd_arr[fd].flags = FILE_IN_USE;

	return fd;
}

/* directory_open 
 *      Inputs: fname - name of file to open, should be "."
 *      Return Value: file descriptor number
 *      Function: find the right array location have the file open, -1 on failure
 *      Side Effects: none     
 */
int32_t directory_open(const uint8_t* fname){
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

	fd_arr[fd].jmp_table = dir_file_ops;
	fd_arr[fd].inode = 0;
	fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file
	fd_arr[fd].flags = FILE_IN_USE;

	return fd;
}

/* file_open 
 *      Inputs: fname - name of file to open
 *      Return Value: file descriptor number
 *      Function: find the right array location have the file open, -1 on failure
 *      Side Effects: none     
 */
int32_t file_open(const uint8_t* fname){
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
	fd_arr[fd].jmp_table = file_file_ops;
	fd_arr[fd].inode = dentry.inode;
	fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file
	fd_arr[fd].flags = FILE_IN_USE;

	return fd;
}

/* rtc_close 
 *      Inputs: fd - file descriptor index value
 *      Return Value: 0 on success, -1 upon failure
 *      Function: attempt to close the rtc in the array and reset it
 *      Side Effects: none     
 */
int32_t rtc_close(int32_t fd){
	// see if the file descriptor index is valid
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX){
		return -1;
	}

	// check if the file is used at all
	if(fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	// reset the file
	fd_arr[fd].jmp_table = NULL;
	fd_arr[fd].inode = -1;
	fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file			
	fd_arr[fd].flags = FILE_NOT_USE;

	// success
	return _rtc_close();
}

/* directory_close 
 *      Inputs: fd - file descriptor index value
 *      Return Value: 0 on success, -1 upon failure
 *      Function: attempt to close the directory in the array and reset it
 *      Side Effects: none     
 */
int32_t directory_close(int32_t fd){
	// see if the file descriptor index is valid
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX){
		return -1;
	}

	// check if the file is used at all
	if(fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	// reset the file
	fd_arr[fd].jmp_table = NULL;
	fd_arr[fd].inode = -1;
	fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file			
	fd_arr[fd].flags = FILE_NOT_USE;

	// success
	return 0;
}

/* file_close 
 *      Inputs: fd - file descriptor index value
 *      Return Value: 0 on success, -1 upon failure
 *      Function: attempt to close the file in the array and reset it
 *      Side Effects: none     
 */
int32_t file_close(int32_t fd){
	// see if the file descriptor index is valid
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX){
		return -1;
	}

	// check if the file is used at all
	if(fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	// reset the file
	fd_arr[fd].jmp_table = NULL;
	fd_arr[fd].inode = -1;
	fd_arr[fd].file_position = 0;			// 0 since we want the beginning of the file			
	fd_arr[fd].flags = FILE_NOT_USE;

	// success
	return 0;
}

/* rtc_write 
 *      Inputs: fd 		- file descriptor index value
 				buf 	- buffer that holds the data to write
 				nbytes 	- how many bytes to write
 *      Return Value: -1 regardless unless rtc in which case, 0
 *      Function: attempt to write to the file, but not implemented for now, 
 					write to rtc if file is of rtc type
 *      Side Effects: none     
 */
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes){
	// sanity checks
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX || fd_arr[fd].flags == FILE_NOT_USE || buf == NULL){
		return -1;
	}

	// if the file is rtc, then we write the buffer into the rtc to change freq
	return _rtc_write(buf);
}

/* directory_write 
 *      Inputs: fd 		- file descriptor index value
 				buf 	- buffer that holds the data to write
 				nbytes 	- how many bytes to write
 *      Return Value: -1 regardless unless rtc in which case, 0
 *      Function: attempt to write to the file, but not implemented for now, 
 					write to rtc if file is of rtc type
 *      Side Effects: none     
 */
int32_t directory_write(int32_t fd, void* buf, int32_t nbytes){
	// return -1 regardless unless rtc
	return -1;
}

/* file_write 
 *      Inputs: fd 		- file descriptor index value
 				buf 	- buffer that holds the data to write
 				nbytes 	- how many bytes to write
 *      Return Value: -1 regardless unless rtc in which case, 0
 *      Function: attempt to write to the file, but not implemented for now, 
 					write to rtc if file is of rtc type
 *      Side Effects: none     
 */
int32_t file_write(int32_t fd, void* buf, int32_t nbytes){
	// return -1 regardless unless rtc
	return -1;
}

/* rtc_read 
 *      Inputs: fd 		- file descriptor index value
 				buf 	- buffer to read into, not used
 				nbytes 	- how many bytes to read
 *      Return Value: 0 on success, -1 upon failure
 *      Function: waits for the rtc to interrupt then returns success
 *      Side Effects: none     
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	// sanity checks
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX || fd_arr[fd].flags == FILE_NOT_USE || fd_arr[fd].jmp_table != rtc_file_ops || buf == NULL){
		return -1;
	}

	// wait for the rtc to interrupt
	_rtc_read();

	// return 0 when success
	return 0;
}

/* directory_read 
 *      Inputs: fd 		- file descriptor index value
 				buf 	- holds the name of the current file we are going to return
 				nbytes 	- how many bytes to read
 *      Return Value: nbytes
 *      Function: reads the current file name and returns it
 *      Side Effects: increments file position so next time we read the following file     
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
	// create buffer to hold the file name
	char dir_name[MAX_NAME_LEN + 1]; 		// length +1 since we want to fir 33 characters here

	// sanity checks
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX || fd_arr[fd].flags == FILE_NOT_USE || fd_arr[fd].jmp_table != dir_file_ops || buf == NULL){
		return -1;
	}

	// see if we have already read all the file directories
	if(fd_arr[fd].file_position == boot_block->num_entries){
		return 0;
	}

	// copy it over to check for '\0' at the end
	strncpy(dir_name, (boot_block->files)[fd_arr[fd].file_position].name, MAX_NAME_LEN);

	// if there isn't a '\0' character at the end, add one
	if(dir_name[MAX_NAME_LEN - 1] != '\0'){
		dir_name[MAX_NAME_LEN] = '\0';
	}

	// bring the current directory name over
	strcpy((char*)buf, dir_name);

	// update the file position
	fd_arr[fd].file_position++;

	return nbytes;
}

/* file_read 
 *      Inputs: fd 		- file descriptor index value
 				buf 	- buffer for where the data goes
 				nbytes 	- how many bytes we want to read
 *      Return Value: how many bytes we read, 0 for end of file, -1 for failure
 *      Function: reads nbytes from the file and fills up the buffer
 *      Side Effects: none
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
	// create variables for function
	int file_len, num_read;
	int offset = fd_arr[fd].file_position;

	// sanity checks
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX || fd_arr[fd].flags == FILE_NOT_USE || fd_arr[fd].jmp_table != file_file_ops || buf == NULL){
		return -1;
	}

	// determine the file length so we don't read too much
	file_len = _get_file_length(fd);

	// check if the offset is valied
	if(offset < 0 || offset >= file_len){
		return 0;
	}

	// perform the read and add to file position if applicable
	num_read = read_data(fd_arr[fd].inode, offset, (uint8_t*)buf, nbytes);
	fd_arr[fd].file_position += num_read;

	// return how many bytes we read
	return num_read;
}

/* read_dentry_by_name 
 *      Inputs: fname 		- name of the file we want to see if it exits
 				dentry 		- if file is valid, we read file info into the dentry
 *      Return Value: 0 on success, -1 on fail
 *      Function: attempts to find the file we are looking for and gives function info in dentry
 *      Side Effects: none
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
	// initialize counter
	int i;
	char curr_file_name[MAX_NAME_LEN + 1];			// length +1 since we want to fir 33 characters here
	char* file_name = (char*)fname;
	dentry_t curr_dentry;

	// sanity checks
	if(fname == NULL || dentry == NULL){
		return -1;
	}

	// loop through all the entries to see which one is the right file
	for(i = 0; i < boot_block->num_entries; i++){
		curr_dentry = (boot_block->files)[i];
		
		// check if the name has the right ending
		if((curr_dentry.name)[MAX_NAME_LEN - 1] != '\0'){ 										// -1 for 0 indexing
			strncpy((int8_t*)curr_file_name, (int8_t*)(curr_dentry.name), MAX_NAME_LEN); 		// copy over the original string
			curr_file_name[MAX_NAME_LEN] = '\0';
		}
		else{
			strcpy((int8_t*)curr_file_name, (int8_t*)(curr_dentry.name)); 						// copy over the original string
		}

		// check if the strings are equal
		if(!(strncmp((int8_t*)curr_file_name, (int8_t*)file_name, strlen((int8_t*)file_name)))){
			// copy over all the pieces in the directory entry
			strcpy((int8_t*)(dentry->name), (int8_t*)curr_dentry.name);
			dentry->type = curr_dentry.type;
			dentry->inode = curr_dentry.inode;
			return 0;
		}
	}

	// we didn't find a matching name, return failure
	return -1;
}

/* read_dentry_by_index 
 *      Inputs: index 		- index of the file that we want to read about
 				dentry 		- dentry to read the information to
 *      Return Value: 0 on success, -1 on fail
 *      Function: attempts to find the file we are looking for and gives function info in dentry
 *      Side Effects: none
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
	dentry_t curr_dentry;

	// if the index is larger, then its a fail
	if(index >= boot_block->num_entries || dentry == NULL){
		return -1;
	}

	// find the correct directory
	curr_dentry = (boot_block->files)[index];

	// copy over all the pieces in the directory entry
	strncmp((int8_t*)(dentry->name), (int8_t*)curr_dentry.name, MAX_NAME_LEN);
	dentry->type = curr_dentry.type;
	dentry->inode = curr_dentry.inode;

	return 0;
}

/* read_data 
 *      Inputs: inode 		- which inode we want to read from
 				offset 		- off in num bytes into the file
 				buf 		- buffer to read the data into
 				length 		- the number of bytes to read from the file from the offset
 *      Return Value: number of bytes we read, -1 on failure
 *      Function: actually goes and finds the data of the file from the filesystem
 *      Side Effects: none
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	// declare the data block for data access, and inode for the inode we are working on
	data_block_t* curr_data_block;
	inode_t curr_inode;

	int data_block_num;										// index to which data block we need to use
	int num_bytes_left; 									// the amount of bytes left to get in case of spillover
	int num_mem2_cpy; 										// how many bytes to copy over
	int num_copied = 0; 									// number of copied bytes
	uint8_t* data_loc;										// pointer to start data location

	// check if the inode number is valid or if buf is valid
	if(inode >= boot_block->num_inode || buf == NULL){
		return -1;
	}

	// otherwise assign the inode
	curr_inode = inodes[inode + 1];

	// more sanity checks, see if offset and length are valid
	if(offset >= curr_inode.length){
		return 0;
	}

	// actually getting the contents with some sanity checks
	num_bytes_left = offset + length >= curr_inode.length ? curr_inode.length - offset : length;
	data_block_num = curr_inode.data_block[offset / BLOCK_SIZE];
	curr_data_block = &(data_ptr[1 + boot_block->num_inode + data_block_num]);
	data_loc = (uint8_t*)curr_data_block + (offset % BLOCK_SIZE);

	// perform memcpy with percautions of spillage
	do{
		// num_mem2_cpy = ((offset + num_copied) % BLOCK_SIZE) + num_bytes_left < BLOCK_SIZE ? num_bytes_left : BLOCK_SIZE - ((offset + num_copied) % BLOCK_SIZE);
		// if we don't have spill over to next data block
		if(((offset + num_copied) % BLOCK_SIZE) + num_bytes_left < BLOCK_SIZE){
			num_mem2_cpy = num_bytes_left;
		}
		else{
			// if we do have spill over, then we get how many we read until the end of the data block
			num_mem2_cpy = BLOCK_SIZE - ((offset + num_copied) % BLOCK_SIZE);
		}
		memcpy((void*)(buf + num_copied), (void*)data_loc, num_mem2_cpy);

		// update trackers
		num_copied += num_mem2_cpy; 			// realized could just use one tracker instead... too late
		num_bytes_left -= num_mem2_cpy;
		data_block_num = curr_inode.data_block[(offset + num_copied) / BLOCK_SIZE];
		curr_data_block = &(data_ptr[1 + boot_block->num_inode + data_block_num]);
		data_loc = (uint8_t*)curr_data_block;
	} while(num_bytes_left > 0);

	// figure out what to return in terms of length or end of file
	return offset + num_copied == curr_inode.length ? 0 : num_copied;
}

/* _get_file_length 
 *      Inputs: fd 		- which file we are looking for the length in the fd_array
 *      Return Value: length fo the file, -1 on failure
 *      Function: finds the length of the file we are looking for
 *      Side Effects: none
 */
int32_t _get_file_length(int32_t fd){
	// sanity checks
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX || fd_arr[fd].flags == FILE_NOT_USE || fd_arr[fd].jmp_table != file_file_ops){
		return -1;
	}

	// check the inode for the length of the file
	return inodes[fd_arr[fd].inode + 1].length;
}
