#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"

#define 	BLOCK_SIZE 		4096
#define 	SLOT_SIZE 		4
#define 	MAX_NAME_LEN	32
#define 	MAX_DIR_ENTRY 	(BLOCK_SIZE / DIR_ENTRY_SIZE) - 1
#define 	MAX_DATA_BLOCK 	(BLOCK_SIZE - SLOT_SIZE) / SLOT_SIZE 	// 1023
#define 	DIR_ENTRY_SIZE	64

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
	dentry 		files[MAX_DIR_ENTRY]
} boot_block_t;

typedef struct file_descriptor{
	uint32_t 	jmp_table;
	uint32_t 	fd_inode;
	uint32_t 	file_position;
	uint32_t 	flags;
} file_descriptor_t;

typedef struct data_block{
	uint8_t 	data[BLOCK_SIZE];
} data_block_t;

// open, close, read, write
// dir_open, file_open

#endif /* _FILESYSTEM_H */