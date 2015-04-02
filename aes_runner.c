#include "user_mmap_driver.h"
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <stdio.h>


#define SHARED_MEM_BASE 0x1F400000

#define SHARED_MEM_LENGTH 0x800000

#define AES_CONTROL_BASE "0x43C00000"

unsigned getAesControlBaseAddress(){
	return strtoul(AES_CONTROL_BASE, NULL, 0);
}

//unsigned getBramInputAddress(){
//	return strtoul(BRAM_INPUT_ADDRESS, NULL, 0);
//}
//
//unsigned getBramOuputAddress(){
//	return strtoul(BRAM_OUTPUT_ADDRESS, NULL, 0);
//}

void writeKey(char* key){
	int i;
	shared_memory key_input = getSharedMemoryArea(getAesControlBaseAddress() + 0x1C, 0);
	if(key_input == NULL){
		printf("Error getting pointer to key");
		return;
	}
	char* aes_key = (char*)key_input->ptr;
	//TODO: for now, assume that we will not go over a page boundary
	//if occurs, write each 4 bytes individually, or check if this will occur
	for(i = 0; i<16; i++){
		aes_key[i] = key[i];
	}
	cleanupSharedMemoryPointer(key_input);
}

void writeKeyValid(){
	writeValueToAddress(0xffffffff, getAesControlBaseAddress() + 0x18);
}

void writeDestinationAddress(int destAddress){
	writeValueToAddress(destAddress, getAesControlBaseAddress() + 0x30);
}

void writeDestinationAddressValid(){
	writeValueToAddress(0xffffffff, getAesControlBaseAddress() + 0x2c);
}

void writeSourceAddress(int sourceAddress){
	writeValueToAddress(sourceAddress, getAesControlBaseAddress() + 0x14);
}

void writeSourceAddressValid(){
	writeValueToAddress(0xffffffff, getAesControlBaseAddress() + 0x10);
}

void writeLength(int length){
	writeValueToAddress(length, getAesControlBaseAddress() + 0x38);
}

void writeLengthValid(){
	writeValueToAddress(0xffffffff, getAesControlBaseAddress() + 0x34);
}

void writeEnable(int enable){
	writeValueToAddress(enable, getAesControlBaseAddress() + 0x40);
}

void writeEnableValid(){
	writeValueToAddress(0xffffffff, getAesControlBaseAddress() + 0x3c);
}

int readFinished(){
	int output;
	getValueAtAddress(getAesControlBaseAddress() + 0x48, &output);
	return output;
}

void signalReadFinished(){
	writeValueToAddress(0xffffffff, getAesControlBaseAddress() + 0x44);
}

int main(){
	int i;
	unsigned char data_to_encrypt[] = {0x01, 0x4B, 0xAF, 0x22, 0x78, 0xA6, 0x9D, 0x33, 0x1D, 0x51, 0x80, 0x10, 0x36, 0x43, 0xE9, 0x9A, '\0'};
	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};

	unsigned char* encrypted_data = (unsigned char*)malloc(16);

	if(encrypted_data == NULL){
		printf("\nCould not allocated encrypted data buffer");
		return -1;
	}
	AES_KEY aes_key;
	AES_set_encrypt_key(key, 128, &aes_key);

	AES_encrypt((unsigned char*)data_to_encrypt, encrypted_data, &aes_key);

	printf("\nEncrypted data as hex: 0x");
	for(i = 0; i<16; i++){
		printf("%02x", encrypted_data[i]);
	}
	printf(" - from openssl\n");
	int source = SHARED_MEM_BASE;
	printf("\nShared memory base: %08x", source);
	int length = SHARED_MEM_LENGTH;
	shared_memory shared_system_mem = getSharedMemoryArea(source, length);
	printf("\nNo segfault");
	if(shared_system_mem == NULL){
		printf("Error getting shared system memory pointer");
		return -1;
	}
	int destOffset = length/2;
//	int dest = source + destOffset;
	int dest = source +1;
	for(i = 0; i<32; i++){
		((char*)shared_system_mem->ptr)[i] = data_to_encrypt[i];
		((char*)shared_system_mem->ptr)[destOffset + i] = 0;
	}
	writeKey(key);
	writeKeyValid();
	writeSourceAddress(source);
	writeSourceAddressValid();
	writeDestinationAddress(dest);
	writeDestinationAddressValid();
	writeLength(1);
	writeLengthValid();
	writeEnable(1);
	writeEnableValid();
	int finished = readFinished();
	printf("\nWaiting for fabric.");
	while(finished == 0){
		printf(".");
		finished = readFinished();
	}
	printf("\n");
	printf("\nData to encrypt as hex: 0x");
	for(i = 0; i<16; i++){
		printf("%02x", ((char*)shared_system_mem->ptr)[i]);
	}
	printf("\n");
	printf("\nEncrypted data as hex: 0x");
	for(i = 0; i<16; i++){
		printf("%02x", ((char*)shared_system_mem->ptr)[destOffset + i]);
	}
	printf(" - from fabric\n");
	return 0;
}

