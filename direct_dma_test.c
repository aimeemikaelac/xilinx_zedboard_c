/*
 * direct_dma_test.c
 *
 *  Created on: Apr 1, 2015
 *      Author: michael
 */

#include "user_mmap_driver.h"
#include "xtest_direct_dma.h"

#define BASE_ADDRESS "0x43C00000"

#define ENABLE_ADDRESS "0x41200000"

#define SHARED_MEM_BASE 0x1F400000

#define SHARED_MEM_LENGTH 0x1000

#define DEVICE_NAME "testdirectdma"

unsigned getBaseAddress(){
	return strtoul(BASE_ADDRESS, NULL, 0);
}

unsigned getEnableAddress(){
	return strtoul(ENABLE_ADDRESS, NULL, 0);
}

void enable(){
	writeValueToAddress(0x1, getBaseAddress());
}

void disable(){
	writeValueToAddress(0x0, getBaseAddress());
}

void writeSourceAddress(int sourceAddress){
	writeValueToAddress(sourceAddress, getBaseAddress() + 0x18);
}

void writeSourceAddressValid(){
	writeValueToAddress(0x1, getBaseAddress() + 0x1c);
}

void writeDestinationAddress(int destAddress){
	writeValueToAddress(destAddress, getBaseAddress() + 0x20);
}

void writeDestinationAddressValid(){
	writeValueToAddress(0x1, getBaseAddress() + 0x24);
}

unsigned getFinished(){
	unsigned output;
	getValueAtAddress(getBaseAddress() + 0x10, &output);
	return output;
}

void clearFinished(){
	writeValueToAddress(3, getBaseAddress() + 0xc);
}

int main(){
	int i;
	int source = SHARED_MEM_BASE;
//	int finished = getFinished();
//	printf("\nFinished value: %i\n", finished);
//	clearFinished();
//	disable();
	printf("\nShared memory base: %08x", source);
	int length = SHARED_MEM_LENGTH;
	printf("\nShared memory length: %08x", length);
	shared_memory shared_system_mem = getSharedMemoryArea(source, 0);
	printf("\nNo segfault");

	//int destOffset = length/2;
	//int dest = source + destOffset;
	for(i = 0; i<length; i++){
		((char*)shared_system_mem->ptr)[i] = 0;
	}

	for(i=0; i<5; i++){
		((char*)shared_system_mem->ptr)[i] = 0xff;
	}

	cleanupSharedMemoryPointer(shared_system_mem);
	printf("\nNo segfault");

	XTest_direct_dma* dma_config = malloc(sizeof(XTest_direct_dma));
	if(XTest_direct_dma_Initialize(dma_config, DEVICE_NAME) != XST_SUCCESS){
		printf("\nError initializing Xilinx uio driver");
		return -1;
	}

	XTest_direct_dma_Set_sourceAddress(dma_config, SHARED_MEM_BASE);
	XTest_direct_dma_Set_destinationAddress(dma_config, SHARED_MEM_BASE + 20);

	XTest_direct_dma_Set_sourceAddress_vld(dma_config);
	XTest_direct_dma_Set_destinationAddress_vld(dma_config);

	XTest_direct_dma_Start(dma_config);

	printf("\nWaiting for fabric");
	while(!XTest_direct_dma_IsDone(dma_config)){
		printf(".");
	}
	int finished = XTest_direct_dma_Get_return(dma_config);
	printf("\nReturn value: %i", finished);

//	enable();
//	writeSourceAddress(SHARED_MEM_BASE);
//	writeDestinationAddress(SHARED_MEM_BASE + 20);
//	writeSourceAddressValid();
//	writeDestinationAddressValid();
//	enable();
//	printf("\nData at source address:");
	printf("\n");
	shared_system_mem = getSharedMemoryArea(source, 0);
	int dummy;
	for(i=0; i<10000000; i++){
		dummy += i*2;
	}
	for(i=0; i<500; i++){
		printf("%02x", ((char*)shared_system_mem->ptr)[i]);
	}
	printf("\nData at dest address:\n");
	for(i=0; i<500; i++){
		printf("%02x", ((char*)shared_system_mem->ptr)[i+10]);
	}
//
//	printf("\nNo segfault");
	printf("\n");
//	finished = getFinished();
//	printf("\nFinished value: %i\n", finished);
//	disable();
//	clearFinished();
	cleanupSharedMemoryPointer(shared_system_mem);
	XTest_direct_dma_Release(dma_config);
//	disable();
}
