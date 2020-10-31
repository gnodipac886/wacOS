#ifndef _SYSTEM_CALLS_H
#define _SYSTEM_CALLS_H

#define STDIN	0
#define STDOUT	1

int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, void* buf, int32_t nbytes);

file_descriptor_t* _get_fd_arr();

#endif /* _FILESYSTEM_H */
