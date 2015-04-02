#include "user_mmap_driver.h"
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <stdio.h>
#include <time.h>


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
	int i, j, output, key_part;
	int current;
//	shared_memory key_input = getSharedMemoryArea(getAesControlBaseAddress() + 0x1C, 0);
//	if(key_input == NULL){
//		printf("Error getting pointer to key");
//		return;
//	}
//	char* aes_key = (char*)(key_input->ptr);
	//TODO: for now, assume that we will not go over a page boundary
	//if occurs, write each 4 bytes individually, or check if this will occur
//	for(i=0; i<16; i++){
//		*aes_key = 0;//key[15-i];
//		aes_key[j] = key[i];
//		aes_key = aes_key + (char)(1);
//	}
/*	aes_key[15] = key[8];
	aes_key[14] = key[8];
	aes_key[13] = key[8];
	aes_key[12] = key[8];
	aes_key[11] = key[8];
	aes_key[10] = key[8];
	aes_key[9] = key[8];
	aes_key[8] = key[8];
	aes_key[7] = key[8];
	aes_key[6] = key[8];
	aes_key[5] = key[8];
	aes_key[4] = key[8];
	aes_key[3] = key[8];
	aes_key[2] = key[8];
	aes_key[1] = key[8];
	aes_key[0] = key[8];*/

	for(i = 0; i<4; i++){
		key_part = 0;
		for(j=0; j<4; j++){
			current = key[15-i*4 - j];
			current = current << (j*8);
			key_part += current;
//		getValueAtAddress(getAesControlBaseAddress() + 0x1c + i/4, &output);
//			printf("\nkey_part %i,%i: %08x", i, j, current);
		}
		writeValueToAddress(key_part, getAesControlBaseAddress() + 0x1c + i*4);
		getValueAtAddress(getAesControlBaseAddress() + 0x1c + i*4, &output);
//		printf("\nkey[%i] is: %08x\n", i, output);
	}
	
//	cleanupSharedMemoryPointer(key_input);
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

int readFinished(){
	int output;
	getValueAtAddress(getAesControlBaseAddress() + 0x40, &output);
	return output;
}

void signalReadFinished(){
	writeValueToAddress(0xffffffff, getAesControlBaseAddress() + 0x3c);
}

int main(){
	int i, j;
	unsigned char data_to_encrypt[] = {0x01, 0x4B, 0xAF, 0x22, 0x78, 0xA6, 0x9D, 0x33, 0x1D, 0x51, 0x80, 0x10, 0x36, 0x43, 0xE9, 0x9A, '\0'};
//	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
	unsigned char data_to_encrypt2[] = {0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, '\0'};
	int data_length = 100000;
	unsigned char data_to_encrypt3[16*data_length];
	unsigned char encrypted_data_openssl[16*data_length];

	srand (time(NULL));

	signalReadFinished();

	int randStart = rand();
	for(i=0; i<data_length; i++){
//		printf("\nNo seg fault: %i", i);
		for(j=0; j<16; j++){
			data_to_encrypt3[16*i + j] = i*j+randStart;
		}
	}

//	unsigned char* encrypted_data = (unsigned char*)malloc(16);

//	if(encrypted_data == NULL){
//		printf("\nCould not allocated encrypted data buffer");
//		return -1;
//	}
	AES_KEY aes_key;
	AES_set_encrypt_key(key, 128, &aes_key);

//	AES_encrypt(data_to_encrypt, encrypted_data, &aes_key);
	

//	printf("\nData to encrypt:\n0x");
//	for(i=0; i<data_length; i++){
//		for(j=0; j<16; j++){		
//			printf("%02x", data_to_encrypt3[i*16 + j]);
//		}
//		printf("  0x");
//	}
//	printf("\nto openssl\n");
//
///	printf("\nEncrypted data as hex:  0x");
//	printf("\n0x");
	unsigned char* data_pointer = data_to_encrypt3;
	unsigned char* encrypted_dest = encrypted_data_openssl;

	clock_t ticks;
	ticks = clock();
	for(i=0; i<data_length; i++){
		AES_encrypt(data_pointer, encrypted_dest, &aes_key);	
//		for(j = 0; j<16; j++){
//			printf("%02x", encrypted_data_openssl[i*16 + j]);
//		}
//		printf("  0x");
		data_pointer = data_pointer + 16;
		encrypted_dest = encrypted_dest + 16;
	}
	ticks = clock() - ticks;

//	printf("\nfrom openssl\n");
	
	printf ("It took %f clicks (%f seconds) in openssl.\n",(float)ticks,((float)ticks)/CLOCKS_PER_SEC);
	int source = SHARED_MEM_BASE;
//	printf("\nShared memory base: %08x", source);
	int length = SHARED_MEM_LENGTH;
	shared_memory shared_system_mem = getSharedMemoryArea(source, length);
//	printf("\nNo segfault");
	if(shared_system_mem == NULL){
		printf("Error getting shared system memory pointer");
		return -1;
	}
	int destOffset = 16*data_length +16;
//	int destOffset = 16;
	int dest = source + destOffset;
//	int dest = source + 16;

//	printf("\nDestination address: %08x\n", dest);
	for(i=0; i<10000; i++){
		((char*)shared_system_mem->ptr)[i] = 0;
	}

//	for(i = 0; i<16; i++){
//		((char*)shared_system_mem->ptr)[i] = data_to_encrypt[i];
//	}
//	
//	for(i=0; i<16; i++){
//		((char*)shared_system_mem->ptr)[i+16] = data_to_encrypt2[i];
//	}
//	printf("\nDestination region before fabric as hex: 0x");
//	for(i = 0; i<16; i++){
//		printf("%02x", ((char*)shared_system_mem->ptr)[i+destOffset]);
//	}
//	printf("\n");


	unsigned int output;
//	for(i = 0; i<4; i++){
//		getValueAtAddress(getAesControlBaseAddress() + 0x1c + i*4, &output);
//		printf("\nkey[%d] read from config register: %08x\n", i, output);
//	}

	
//	for(i=0; i<4; i++){
//		getValueAtAddress(source + i*4, &output);
//		printf("\nUnencrypted Data[%i] in memory: %08x", i, output);
//	}

	unsigned int current, currentTotal, k;
/*	for(i=0; i<100; i++){
		current = 0;
		for(j = 0; j<4; j++){
			currentTotal = 0;
			for(k=0; k<4; k++){
				current += data_to_encrypt3[i*16 + j*4 + k];
				current = current << (8*k);
				currentTotal += current;
//			((char*)shared_system_mem->ptr)[i] = data_to_encrypt3[i*16 + j];
//			printf("%02x", ((char*)shared_system_mem->ptr)[i*16 + j]);
			}
			writeValueToAddress(currentTotal, SHARED_MEM_BASE + i*4 + j);
			getValueAtAddress(SHARED_MEM_BASE + i*4 + j, &output);
			printf("%08x ", output);
		}
	}*/
	
//	printf("\nData to encrypt as hex:\n0x");
	for(i=0; i<data_length; i++){
		for(j=0; j<16; j++){
			((char*)shared_system_mem->ptr)[i*16 +j] = data_to_encrypt3[i*16 + j];
//			printf("%02x", ((char*)shared_system_mem->ptr)[i*16 +j]);
		}
//		printf("  0x");
	}
//
//	printf("\nto fabric\n");

	cleanupSharedMemoryPointer(shared_system_mem);
	writeKey(key);
	writeSourceAddress(source);
	writeDestinationAddress(dest);
	writeLength(data_length);
	
	ticks = clock();
	writeLengthValid();
	writeDestinationAddressValid();
	writeSourceAddressValid();
	writeKeyValid();

	int finished = readFinished();
	printf("\nWaiting for fabric.");
	while(finished == 0){
		printf(".");
		finished = readFinished();
	}
	ticks = clock() - ticks;
	printf("\nDone waiting for fabric\n");
//	for(i = 0; i<16; i++){
//		printf("%02x", ((char*)shared_system_mem->ptr)[16+i]);
//	}
//	printf("\n");
	shared_system_mem = getSharedMemoryArea(source, length);
//	printf("\nNo segfault");
	if(shared_system_mem == NULL){
		printf("Error getting shared system memory pointer");
		return -1;
	}
/*	printf("\nWaiting...");
	int dummy = 0;
	for(i=0; i<data_length; i++){
//		printf(".");
		for(j=0; j<1000; j++){
			dummy += i*j;
		}
	}
	printf("\n");*/
	printf ("It took %f clicks (%f seconds) in fabric.\n",(float)ticks,((float)ticks)/CLOCKS_PER_SEC);
//	printf("\nEncrypted data as hex:\n0x");

//	for(i = 0; i<data_length; i++){
//		for(j=0; j<16; j++){
//			char current =  ((char*)shared_system_mem->ptr)[i*16 + j + destOffset];
//			printf("%02x", current);
//		}
//		printf("  0x");
//	}
//	printf("\nfrom fabric\n");
//	for(i = 0; i<16; i++){
//		char current =  ((char*)shared_system_mem->ptr)[16 + i + destOffset];
//		printf("%02x", current);
//	}
//	printf(" - from fabric\n");
/*	for(i=0; i<4; i++){
		getValueAtAddress(dest + i*4, &output);
		if(output != 0){
			printf("\n%08x at %i", output, i);
		}
	}
	printf("\n");*/
	int incorrectCount = 0;
	for(i=0; i<data_length; i++){
		for(j=0; j<16; j++){
			char openssl = encrypted_data_openssl[i*16 + j];
			char fabric = ((char*)shared_system_mem->ptr)[i*16 + j + destOffset];
			if(openssl != fabric){
//				printf("\nChar at index %i is not encrypted correctly. It is %02x in openssl, %02x in fabric", i*16+j, openssl, fabric);
				incorrectCount++;
			}
		}
	}
	printf("\nNum incorrect: %i\n", incorrectCount);
	cleanupSharedMemoryPointer(shared_system_mem);
	signalReadFinished();
	return 0;
}

