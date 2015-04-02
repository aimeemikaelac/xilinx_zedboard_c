/*
 * direct_dma_test.c
 *
 *  Created on: Apr 1, 2015
 *      Author: michael
 */

#include "user_mmap_driver.h"

#define BASE_ADDRESS "0x43C00000"

#define SHARED_MEM_BASE 0x1F400000

#define SHARED_MEM_LENGTH 0x800000

unsigned getBaseAddress(){
	return strtoul(BASE_ADDRESS, NULL, 0);
}

void writeSourceAddress(int sourceAddress){
	writeValueToAddress(sourceAddress, getBaseAddress() + 0x14);
}

void writeSourceAddressValid(){
	writeValueToAddress(0xffffffff, getBaseAddress() + 0x10);
}

void writeDestinationAddress(int destAddress){
	writeValueToAddress(destAddress, getBaseAddress() + 0x1c);
}

void writeDestinationAddressValid(){
	writeValueToAddress(0xffffffff, getBaseAddress() + 0x18);
}

int main(){
	int i;
	int source = SHARED_MEM_BASE;
	printf("\nShared memory base: %08x", source);
	int length = SHARED_MEM_LENGTH;
	printf("\nShared memory length: %08x", length);
	shared_memory shared_system_mem = getSharedMemoryArea(source, 0);
	printf("\nNo segfault");

	//int destOffset = length/2;
	//int dest = source + destOffset;
	for(i = 0; i<100; i++){
		((char*)shared_system_mem->ptr)[i] = 0;
	}

	((char*)shared_system_mem->ptr)[0] = 0xff;
	printf("\nNo segfault");
	writeSourceAddress(SHARED_MEM_BASE);
	writeDestinationAddress(SHARED_MEM_BASE + 1);
	writeSourceAddressValid();
	writeDestinationAddressValid();
	for(i=0; i<0xff; i++){
		((char*)shared_system_mem->ptr)[0] = (char)i;
		printf("\nData at source address: %02x", ((char*)shared_system_mem->ptr)[0]);
		printf("\nData at dest address: %02x", ((char*)shared_system_mem->ptr)[1]);
		writeSourceAddressValid();
		writeDestinationAddressValid();
		printf("\nData at source address: %02x - fabric", ((char*)shared_system_mem->ptr)[0]);
		printf("\nData at dest address: %02x - fabric", ((char*)shared_system_mem->ptr)[1]);
	}

	printf("\nNo segfault");
	printf("\n");
	cleanupSharedMemoryPointer(shared_system_mem);
}
