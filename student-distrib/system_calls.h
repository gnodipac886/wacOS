#ifndef _SYSTEM_CALLS_H
#define _SYSTEM_CALLS_H

#include "types.h"
#include "filesystem.h"

#define STDIN				0
#define STDOUT				1
#define PG_4MB 				0x400000
#define FRST_INSTR			23
#define KER_TOP 			0x400000 			// 4MB
#define KER_BOTTOM			0x800000 			// 8MB
#define KER_STACK_SIZE		0x2000				// 8KB
#define PCB_MASK 			0xFFFFE000
#define USR_PTR 			0x8048000 			// 128 MB
#define MAX_TASKS           6

typedef struct pcb{
	char 	arg[128];
	int32_t pid;
	int32_t parent_pid;
} pcb_t;

int32_t execute(const uint8_t* command);
int32_t halt(uint8_t status);
int32_t open(const uint8_t* fname);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, void* buf, int32_t nbytes);
int32_t close(int32_t fd);


int32_t invalid_func();
file_descriptor_t* _get_fd_arr();
pcb_t* _get_curr_pcb(int32_t* ptr);

#endif /* _SYSTEM_CALLS_H */
