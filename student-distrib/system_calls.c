#include "system_calls.h"
#include "filesystem.h"
#include "types.h"

/*
user:
ece391open(".")

ece391syscall.S 
pushes args into registers
int $80

idt.c
skip syscallhandler and go straight to assembly linkage

assembly linkage
push arguments onto stack all 3 is fine

say open func:
switch statements for particular open (rtc, files, dir)

say rtc_open:
code



*/


int32_t open(const uint8_t* fname){
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


	switch (dentry.type) {
		case RTC_TYPE:
			rtc_open(fname);
			break;
		case DIR_TYPE:
			directory_open(fname);
			break;
		case FILE_TYPE:
			file_open(fname);
			break;
		default:
			return -1;
	}

	return fd;
}


int32_t read(){

}

int32_t write(){

}

int32_t close(const uint8_t* fname){
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


	// see if the file descriptor index is valid
	if(fd >= MAX_FILES_OPEN || fd < FIRST_FILE_IDX){
		return -1;
	}

	// check if the file is used at all
	if(fd_arr[fd].flags == FILE_NOT_USE){
		return -1;
	}


	switch (dentry.type) {
		case RTC_TYPE:
			rtc_close(fd);
			break;
		case DIR_TYPE:
			directory_close(fd);
			break;
		case FILE_TYPE:
			file_close(fd);
			break;
		default:
			return -1;
	}

	return fd;
}