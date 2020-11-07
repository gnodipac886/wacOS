#ifndef _SYSTEM_CALLS_H
#define _SYSTEM_CALLS_H

#include "types.h"
#include "filesystem.h"

#define STDIN				0
#define STDOUT				1
#define KER_BOTTOM			0x800000 			// 8MB
#define KER_STACK_SIZE		0x2000				// 8KB
#define USR_PTR 			0x8000000 			// 128 MB

typedef struct pcb{
	char 	arg[128];
	int32_t pid;
	int32_t parent_pid;
} pcb_t;

int32_t open(const uint8_t* fname);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, void* buf, int32_t nbytes);
int32_t close(int32_t fd);


int32_t invalid_func();
file_descriptor_t* _get_fd_arr();

#endif /* _SYSTEM_CALLS_H */
