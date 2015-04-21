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
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct shared_memory{
	int fd;
	volatile void* ptr;
	volatile void* original_ptr;
	unsigned length;
	unsigned offset;
	unsigned page_size;
} shared_memory_struct;

typedef shared_memory_struct* shared_memory;

//get an int at an address in system memory
int getValueAtAddress(unsigned address, unsigned int* value);
//write an int to an address in system memory
int writeValueToAddress(unsigned int value, unsigned address);
//get a pointer to a memory region of a certain length through /dev/mem
shared_memory getSharedMemoryArea(unsigned address, unsigned length);
//cleanup a shared_memory struct
void cleanupSharedMemoryPointer(shared_memory mem);
//sync a shred_memory struct
//int syncSharedMemory(shared_memory mem);

shared_memory getUioMemoryArea();
int writeValueToAddressUio(unsigned int value, int offset);
int getValueAtAddressUio(int offset, unsigned int* value);

#endif /* USER_MMAP_DRIVER_H_ */
