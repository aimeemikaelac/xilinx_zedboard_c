#include "stdio.h"
#include <openssl/rand.h>
#include <openssl/aes.h>
#include "memmgr.h"
#include "user_mmap_driver.h"
#include "aes_fpga.h"

int main(){
	int i;
	unsigned shared_size = 0x70000; //4*1024*1024;
	unsigned base_address = 0x1f410000;
	char* sharedUioDevice = "/dev/uio1";
	char* data1;
	char* data2;
	char* encrypted_dest;
	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};

	AES_KEY aes_key;
	AES_set_encrypt_key(key, 128, &aes_key);

	printf("\n--------------------------------------------\n");

	int numBytes = 16*64;

//	shared_memory shared_mem = getUioMemoryArea(sharedUioDevice, shared_size);
	shared_memory shared_mem = getSharedMemoryArea(base_address, shared_size);
	printf("\nShared mem pointer: %p", shared_mem->ptr);
	for(i=0; i<shared_size; i++){
		((char*)shared_mem->ptr)[i] = 0xff;
	}
//	memmgr_init((void*)(shared_mem->ptr), shared_size, base_address);

	char* shared = ((char*)shared_mem->ptr);

	data1 = shared;
//	data1 = (char*)memmgr_alloc(1024);
	data2 = shared+1024;
//	data2 = (char*)memmgr_alloc(1024);
	encrypted_dest = shared+2048;
//	encrypted_dest = (char*)memmgr_alloc(1024);

//	unsigned data1_address = lookupBufferPhysicalAddress(data1);
//	unsigned data2_address = lookupBufferPhysicalAddress(data2);
//	unsigned encrypted_dest_address = lookupBufferPhysicalAddress(encrypted_dest);

//	printf("\nData1 address: 0x%08x", data1_address);
//	printf("\nData2 address: 0x%08x", data2_address);
//	printf("\nEncrypted dest address: 0x%08x", encrypted_dest_address);
	srand(time(NULL));

	int randStart = rand();
	char current;
	for(i=0; i<1024; i++){
		current = (char)('a' + ((char)(i))/16 + ((char)randStart));
		data1[i] = current;
	}
/*
//	data1[0]='\0';
//	for(i=1023; i>0; i-= 35){
//		strncat(data1, "Now is the time for all good onions", i);
//	}
	
	printf("\nStarting input");
	for(i=0; i<16; i++){
		printf("\n0x%02x", data1[i]);
	}

	for(i=0; i<1024; i++){
		data2[i] = 'A';
		encrypted_dest[i] = 'A';
	}


	printf("\nStarting output");
	printf("\nFPGA\t|\tOpenSSL");
	for(i=0; i<16; i++){
		printf("\n0x%02x\t|\t0x%02x", data2[i], encrypted_dest[i]);
	}

	printf("\n");*/


	FPGA_AES *cipher1 = NULL;
	if((cipher1 = fpga_aes_new(key, 16, base_address, "aes-qam", "axi-reset")) == NULL){
		printf("\nCould not allocate cipher1");
		abort();
	}

//	byteReverseBuffer16(data1, 1024);
	unsigned data1addr = (unsigned)(data1) - (unsigned)(shared) + base_address;
	unsigned data2addr = (unsigned)(data2) - (unsigned)(shared) + base_address;
	unsigned encrypted_destaddr = (unsigned)(encrypted_dest) - (unsigned)(shared) + base_address;
	printf("\nSource address: 0x%08x", base_address);
	printf("\nDestination address: 0x%08x", base_address + 1024);
	printf("\nData1 addr: 0x%08x", data1addr);
	printf("\nData2 addr: 0x%08x", data2addr);
	printf("\nEncrypted_dest addr: 0x%08x", encrypted_destaddr);
//	aes_encrypt(cipher1, numBytes, base_address, base_address + 1024);
	Aes_encrypt_run(cipher1, data1, 1024, data2, base_address, base_address + 1024);
//	Aes_encrypt_memmgr(cipher1, data2, data1, 16);
//	byteReverseBuffer16(data2, 1024);
	for(i=0; i<numBytes/16; i++){
		AES_encrypt(data1+i*16, encrypted_dest+i*16, &aes_key);
	}
//	AES_encrypt(data1+16, encrypted_dest+16, &aes_key);
	
	printf("\n--------------------------------------------\n");
	printf("\nFPGA\t|\tOpenSSL");
	int countIncorrect = 0;
	for(i=0; i<numBytes; i++){
		if(numBytes <= 32){
			if(i%16 == 0){
				printf("\n--------------------------------------------");
			}
			printf("\n0x%02x\t|\t0x%02x", data2[i], encrypted_dest[i]);
		} 
		if(data2[i] != encrypted_dest[i]){
			countIncorrect++;
		}
	}
	printf("\nNum incorrect: %i", countIncorrect);
	printf("\nNum total: %i", numBytes);
	printf("\n");

//	memmgr_free(data1);
//	memmgr_free(data2);
//	memmgr_free(encrypted_dest);
	fpga_aes_free(cipher1);
	cleanupSharedMemoryPointer(shared_mem);
}
