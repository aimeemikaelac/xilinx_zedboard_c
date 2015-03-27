/*
 * user_mmap_driver.h
 *
 *  Created on: Mar 26, 2015
 *      Author: michael
 */

#ifndef USER_MMAP_DRIVER_H_
#define USER_MMAP_DRIVER_H_

#include <sys/mman.h>
#include <fcntl.h>

typedef struct shared_memory{
	int fd;
	void* ptr;
	void* original_ptr;
	unsigned length;
	unsigned offset;
	unsigned page_size;
} shared_memory_struct;

typedef shared_memory_struct* shared_memory;

//get an int at an address in system memory
int getValueAtAddress(unsigned address, int* value);
//write an int to an address in system memory
int writeValueToAddress(int value, unsigned address);
//get a pointer to a memory region of a certain length through /dev/mem
shared_memory getSharedMemoryArea(unsigned address, unsigned length);



#endif /* USER_MMAP_DRIVER_H_ */
