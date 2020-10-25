#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"

#define 	BLOCK_SIZE 		4096
#define 	SLOT_SIZE 		4
#define 	MAX_NAME_LEN	32
#define 	MAX_DIR_ENTRY 	(BLOCK_SIZE / DIR_ENTRY_SIZE) - 1
#define 	MAX_DATA_BLOCK 	(BLOCK_SIZE - SLOT_SIZE) / SLOT_SIZE 	// 1023
#define 	DIR_ENTRY_SIZE	64

#define 	MAX_FILES_OPEN 	8
#define 	FIRST_FILE_IDX 	2

#define 	RTC_TYPE		0
#define 	DIR_TYPE 		1
#define 	FILE_TYPE 		2

#define 	FILE_IN_USE 	1
#define 	FILE_NOT_USE 	0

typedef struct dentry{
	char 		name[MAX_NAME_LEN];
	uint32_t 	type;
	uint32_t 	inode;
	uint8_t 	reserved[24];
} dentry_t;

typedef struct inode{
	uint32_t 	length;
	uint32_t 	data_block[MAX_DATA_BLOCK];
} inode_t;

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

typedef struct data_block{
	uint8_t 	data[BLOCK_SIZE];
} data_block_t;

void __init_filesystem__(void * filesystem_ptr);
int32_t _open(const uint8_t* fname);
int32_t _close(int32_t fd);
int32_t _write(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t _get_file_length(int32_t fd);

#endif /* _FILESYSTEM_H */
