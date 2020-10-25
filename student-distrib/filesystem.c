#include "filesystem.h"
#include "lib.h"

static void * filesystem;
boot_block_t* boot_block;
inode_t* inodes;
data_block_t* data_ptr;

file_descriptor_t fd_arr[MAX_FILES_OPEN];

uint32_t rtc_file_ops[4] = {(uint32_t)_open, (uint32_t)rtc_read, (uint32_t)_write, (uint32_t)_close};
uint32_t dir_file_ops[4] = {(uint32_t)_open, (uint32_t)dir_read, (uint32_t)_write, (uint32_t)_close};
uint32_t file_file_ops[4] = {(uint32_t)_open, (uint32_t)file_read, (uint32_t)_write, (uint32_t)_close};


void __init_filesystem__(void * filesystem_ptr){
	int i;
	filesystem 	= filesystem_ptr;
	boot_block 	= (boot_block_t*)filesystem;
	inodes 		= (inode_t*)filesystem;
	data_ptr 	= (data_block_t*)filesystem;

	// set up the file descriptor array
	for(i = 0; i < MAX_FILES_OPEN; i++){
		fd_arr[i].jmp_table = NULL;
		fd_arr[i].inode = -1;
		fd_arr[i].file_position = -1;
		fd_arr[i].flags = FILE_NOT_USE;
	}
}

int32_t _open(const uint8_t* fname){
	int i;
	int fd = FIRST_FILE_IDX;
	dentry_t dentry;

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

	// loop through the array to see which 
	while(fd_arr[fd].flags){
		fd++;

		// if the whole file array is full, return fail
		if(fd >= 8){
			return -1;
		}
	}

	// set the jump table for the file operations
	switch(dentry.type){
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

	// set up the rest of the file descriptor
	fd_arr[fd].inode = dentry.inode;

	if(dentry.type == RTC_TYPE || dentry.type == DIR_TYPE){
		fd_arr[fd].inode = 0;
	}

	fd_arr[fd].file_position = 0;
	fd_arr[fd].flags = FILE_IN_USE;

	return fd;
}

int32_t _close(int32_t fd){
	// see if the file descriptor index is valid
	if(fd >= 8 || fd < 2){
		return -1;
	}

	// check if the file is used at all
	if(fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	// reset the file
	fd_arr[fd].jmp_table = NULL;
	fd_arr[fd].inode = -1;
	fd_arr[fd].file_position = -1;
	fd_arr[fd].flags = FILE_NOT_USE;

	return 0;
}

// needs to complete
int32_t _write(int32_t fd, void* buf, int32_t nbytes){
	if(fd >= 8 || fd < 2 || fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}

	if(fd_arr[fd].jmp_table == rtc_file_ops){
		// call rtc write here

		return 0;
	}

	return -1;
}

// needs to complete
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	// add some black magic here to wait for an interrupt to happen
	return 0;
}

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
	char dir_name[MAX_NAME_LEN + 1];

	// sanity checks
	if(fd >= 8 || fd < 2 || fd_arr[fd].flags == FILE_NOT_USE || fd_arr[fd].jmp_table != dir_file_ops){
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

int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
	int offset = 0;

	if(fd >= 8 || fd < 2 || fd_arr[fd].flags == FILE_NOT_USE || fd_arr[fd].jmp_table != file_file_ops){
		return -1;
	}

	return read_data(fd_arr[fd].inode, offset, (uint8_t*)buf, nbytes);
}

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
	// initialize counter
	int i;
	char curr_file_name[33];
	char* file_name = (char*)fname;
	dentry_t curr_dentry;

	// loop through all the entries to see which one is the right file
	for(i = 0; i < boot_block->num_entries; i++){
		curr_dentry = (boot_block->files)[i];
		// curr_file_name = _get_file_name(curr_dentry.name);
		
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

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
	dentry_t curr_dentry;

	// if the index is larger, then its a fail
	if(index >= boot_block->num_entries){
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

// actually does the data retrieval
// remember when using this, make sure it's the right file type, not dir or RTC
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	data_block_t* curr_data_block;
	inode_t curr_inode;

	int data_block_num;										// index to which data block we need to use
	// int num_spill_over; 									// number of spill over data blocks (how many data blocks do we need to look at in total)
	int num_bytes_left; 									// the amount of bytes left to get in case of spillover
	int num_mem2_cpy; 										// how many bytes to copy over
	int num_copied = 0; 									// number of copied bytes
	uint8_t* data_loc;										// pointer to start data location

	// check if the inode number is valid
	if(inode >= boot_block->num_inode){
		return -1;
	}

	// otherwise assign the inode
	curr_inode = inodes[inode + 1];

	// more sanity checks, see if offset and length are valid
	if(offset >= curr_inode.length){
		return -1;
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

	// figure out what to return
	return offset + num_copied == curr_inode.length ? 0 : num_copied;
}

int32_t _get_file_length(int32_t fd){
	if(fd >= 8 || fd < 2 || fd_arr[fd].flags == FILE_NOT_USE || fd_arr[fd].jmp_table != file_file_ops){
		return -1;
	}

	return inodes[fd_arr[fd].inode + 1].length;
}
