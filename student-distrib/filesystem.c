#include "filesystem.h"
#include "lib.h"

static void * filesystem;
file_descriptor_t fd_arr[8];

void __init_filesystem__(void * filesystem_ptr){
	filesystem = filesystem_ptr;
}

//!!!!!!!!!!!!!!!!!!!! make sure fname is \0 terminated               !!!!!!!!!!!!!!!!!!!!!!!!!!!
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
	// initialize counter
	int i;
	boot_block_t* boot_block = (boot_block_t*)filesystem;
	char* curr_file_name;
	char* file_name = (char*)fname;
	dentry curr_dentry;

	// loop through all the entries to see which one is the right file
	for(i = 0; i < boot_block->num_entries; i++){
		curr_dentry = (boot_block->files)[i];
		curr_file_name = _get_file_name(curr_dentry.name);

		// check if the strings are equal
		if(!(strncmp((int8_t*)curr_file_name, (int8_t*)file_name, strlen((int8_t*)file_name)))){
			// copy over all the pieces in the directory entry
			strncmp((int8_t*)(dentry->name), (int8_t*)curr_dentry.name, MAX_NAME_LEN);
			dentry->type = curr_dentry.type;
			dentry->inode = curr_dentry.inode;
			return 0;
		}
	}

	// we didn't find a matching name, return failure
	return -1;
}

int32_t read_dentry_by_index(uint32_t index, dentry* dentry){
	dentry curr_dentry;
	boot_block_t* boot_block = (boot_block_t*)filesystem;

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
}

// actually does the data retrieval
// remember when using this, make sure it's the right file type, not dir or RTC
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	boot_block_t* boot_block = (boot_block_t*)filesystem;
	inode_t* inodes = (inode_t*)filesystem;
	data_block_t* data_ptr = (data_block_t*)filesystem;
	data_block_t* curr_data_block;
	inode_t curr_inode;

	int data_block_idx;										// index to which data block we need to use
	// int num_spill_over; 									// number of spill over data blocks (how many data blocks do we need to look at in total)
	int num_bytes_left = length; 							// the amount of bytes left to get in case of spillover
	int num_mem2_cpy; 										// how many bytes to copy over
	int num_copied = 0; 									// number of copied bytes
	uint8_t* data_loc;										// pointer to start data location

	// check if the inode number is valid
	if(inode >= boot_block->num_inode){
		return -1;
	}

	// otherwise assign the inode
	curr_inode = inodes[inode];

	// more sanity checks, see if offset and length are valid
	if(offset >= curr_inode.length || length >= curr_inode.length){
		return -1;
	}

	// actually getting the contents with some sanity checks
	data_block_idx = offset / BLOCK_SIZE;
	curr_data_block = &(data_ptr[1 + boot_block->num_inode + data_block_idx]);
	data_loc = (uint8_t*)curr_data_block + (offset % BLOCK_SIZE);

	// perform memcpy with percautions of spillage
	do{
		// num_mem2_cpy = ((offset + num_copied) % BLOCK_SIZE) + num_bytes_left < BLOCK_SIZE ? num_bytes_left : BLOCK_SIZE - ((offset + num_copied) % BLOCK_SIZE);
		// if we don't have spill over to next data block
		if(((offset + num_copied) % BLOCK_SIZE) + num_bytes_left < BLOCK_SIZE){
			num_mem2_cpy = num_bytes_left;
		}
		else{
			num_mem2_cpy = BLOCK_SIZE - ((offset + num_copied) % BLOCK_SIZE)
		}
		memcpy((void*)(buf + num_copied), (void*)data_loc);

		// update trackers
		num_copied += num_mem2_cpy; 			// realized could just use one tracker instead... too late
		num_bytes_left -= num_mem2_cpy;
		data_block_idx++;
		curr_data_block = &(data_ptr[1 + boot_block->num_inode + data_block_idx]);
		data_loc = 0;
	} while(num_bytes_left > 0);

	// figure out what to return
	return offset + num_copied == curr_inode.length ? 0 : num_copied;
}



char* _get_file_name(char* fname){
	// counter for for loop
	int i;
	char* name[33];

	// check if the name is 32 characters long by checking the last character
	if(fname[MAX_NAME_LEN - 1] != '\0'){ 							// -1 for 0 indexing
		strncpy((int8_t*)name, (int8_t*)fname, MAX_NAME_LEN); 		// copy over the original string
		name[32] = '\0';
		return name;
	}

	return fname;
}