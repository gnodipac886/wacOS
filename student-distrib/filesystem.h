#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"

#define 	BLOCK_SIZE 		4096									// size of each block in filesystem
#define 	SLOT_SIZE 		4										// 4 bytes 
#define 	MAX_NAME_LEN	32										// max name length for file 
#define 	MAX_DIR_ENTRY 	(BLOCK_SIZE / DIR_ENTRY_SIZE) - 1 		// 63
#define 	MAX_DATA_BLOCK 	(BLOCK_SIZE - SLOT_SIZE) / SLOT_SIZE 	// 1023
#define 	DIR_ENTRY_SIZE	64										// size of each directory

#define 	MAX_FILES_OPEN 	8										// maximum number of files open at the same time
#define 	FIRST_FILE_IDX 	2										// first available file index 

#define 	RTC_TYPE		0										// rtc type file number 
#define 	DIR_TYPE 		1										// directory type file number
#define 	FILE_TYPE 		2										// regular file type number

#define 	FILE_IN_USE 	1										// if file is in use 
#define 	FILE_NOT_USE 	0										// if file is not in use 

// data structure for dentry
typedef struct dentry{
	char 		name[MAX_NAME_LEN];
	uint32_t 	type;
	uint32_t 	inode;
	uint8_t 	reserved[24];
} dentry_t;

// data structure for inode
typedef struct inode{
	uint32_t 	length;
	uint32_t 	data_block[MAX_DATA_BLOCK];
} inode_t;

// data structure for boot block
typedef struct boot_block{
	uint32_t 	num_entries;
	uint32_t 	num_inode;
	uint32_t 	num_data_block;
	uint8_t 	reserved[52];
	dentry_t 	files[MAX_DIR_ENTRY];
} boot_block_t;

typedef struct file_descriptor{
	uint32_t* 	jmp_table;
	uint32_t 	inode;
	uint32_t 	file_position;
	uint32_t 	flags;					// 1 for in use, 0 for vacant
} file_descriptor_t;

// data structure for data block 
typedef struct data_block{
	uint8_t 	data[BLOCK_SIZE];
} data_block_t;

typedef struct f_ops_jmp_table{
	uint32_t*	f_ops_open;
	uint32_t*	f_ops_read;
	uint32_t*	f_ops_write;
	uint32_t*	f_ops_close;
} f_ops_jmp_table_t;

void __init_filesystem__(void * filesystem_ptr);

int32_t rtc_open(const uint8_t* fname);
int32_t directory_open(const uint8_t* fname);
int32_t file_open(const uint8_t* fname);

int32_t rtc_close(int32_t fd);
int32_t directory_close(int32_t fd);
int32_t file_close(int32_t fd);

int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_write(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, void* buf, int32_t nbytes);

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t _get_file_length(int32_t fd);

#endif /* _FILESYSTEM_H */
