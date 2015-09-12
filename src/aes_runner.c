#include "user_mmap_driver.h"
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <stdio.h>
#include <time.h>
#include "xaes.h"

#define SHARED_MEM_BASE 0x1F410000

#define SHARED_MEM_LENGTH 0x30000

#define AES_CONTROL_BASE "0x43C00000"


unsigned char reverse_char(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

uint32_t reverse(uint32_t x)
{
	x = ((x >> 1) & 0x55555555u) | ((x & 0x55555555u) << 1);
	x = ((x >> 2) & 0x33333333u) | ((x & 0x33333333u) << 2);
	x = ((x >> 4) & 0x0f0f0f0fu) | ((x & 0x0f0f0f0fu) << 4);
	x = ((x >> 8) & 0x00ff00ffu) | ((x & 0x00ff00ffu) << 8);
	x = ((x >> 16) & 0xffffu) | ((x & 0xffffu) << 16);
	return x;
}

char *int2bin(int a, char *buffer, int buf_size) {
	buffer += (buf_size - 1);
	int i;
	for (i = 31; i >= 0; i--) {
		*buffer-- = (a & 1) + '0';
		
		a >>= 1;
	}
	
	return buffer;
}

int main(int argc, char** argv){
	int i, j;
	unsigned char data_to_encrypt[] = {0x01, 0x4B, 0xAF, 0x22, 0x78, 0xA6, 0x9D, 0x33, 0x1D, 0x51, 0x80, 0x10, 0x36, 0x43, 0xE9, 0x9A, '\0'};
//	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
	unsigned char data_to_encrypt2[] = {0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, '\0'};
	int data_length;
	if(argc == 2){
		data_length = atoi(argv[1]);
	} else{
		data_length = 100;
	}
	printf("Data length: %i\n", data_length);
	unsigned char data_to_encrypt3[16*data_length];
	unsigned char encrypted_data_openssl[16*data_length];

	srand (time(NULL));

	int randStart = rand();
	for(i=0; i<data_length; i++){
		for(j=0; j<16; j++){
			data_to_encrypt3[16*i + j] = i*j+3;//+randStart;
		}
	}

	AES_KEY aes_key;
	AES_set_encrypt_key(key, 128, &aes_key);

	unsigned char* data_pointer = data_to_encrypt3;
	unsigned char* encrypted_dest = encrypted_data_openssl;

	clock_t begin, end;
	begin = clock();
	for(i=0; i<data_length; i++){
		AES_encrypt(data_pointer, encrypted_dest, &aes_key);	
		data_pointer = data_pointer + 16;
		encrypted_dest = encrypted_dest + 16;
	}
	end = clock();
	double ticks = (double)(end - begin);
	double seconds =(double)(end - begin)/CLOCKS_PER_SEC;
	printf ("It took %f clicks (%f seconds) in openssl.\n",ticks,seconds);
	FILE *openssl_fabric_log;
	openssl_fabric_log = fopen("aes_openssl_results.csv", "a");
	fprintf(openssl_fabric_log, "%f,%i\n", seconds, data_length);
	fclose(openssl_fabric_log);
	int source = SHARED_MEM_BASE;
	int length = SHARED_MEM_LENGTH;
	shared_memory shared_system_mem = getUioMemoryArea("/dev/uio1",0x80000);//getSharedMemoryArea(source, length);//getUioMemoryArea("/dev/uio1", length);//=
	if(shared_system_mem == NULL){
		printf("Error getting shared system memory pointer");
		return -1;
	}
	int destOffset = 16*data_length +16;
	int dest = source + destOffset;

	for(i=0; i<0x1000; i++){
		((char*)shared_system_mem->ptr)[i] = 0;
	}

	unsigned int output;
	unsigned int current, currentTotal, k;
	for(i=0; i<data_length; i++){
		for(j=0; j<16; j++){
			((char*)shared_system_mem->ptr)[i*16 + (15-j)] = data_to_encrypt3[i*16 + j];
		}
	}

//	cleanupSharedMemoryPointer(shared_system_mem);
	XAes aes_device_struct;// = NULL;
	XAes *aes_device = &aes_device_struct;
//	aes_device = malloc(sizeof(XAes));
//	if(aes_device == NULL){
//		printf("\nCould not allocate memory for aes device");
//		return -1;
//	}
//	if(XAes_Initialize(aes_device, "aestest") != XST_SUCCESS){
//	printf("Test3------------------------------------\n");
	if(XAes_Initialize(aes_device, "aes-qam") != XST_SUCCESS){
		printf("\nCould not initialize aes device");
		return -1;
	}

//	printf("Test------------------------------------\n");
	XAes_Key_in_v key_in;// = NULL;
//	key_in = (XAes_Key_in_v*)malloc(sizeof(XAes_Key_in_v));
//	if(key_in == NULL){
//		printf("\nCould not allocate memory for key struct");
//		return -1;
//	}
	u32 key_array[4];
	for(i=0; i<4; i++){
		u32 current = 0;
		for(j=0; j<4; j++){
			int key_part = key[15-i*4-j];
//			char current_char = key[i*4+j];
//			current_char = reverse(current_char);
//			int key_part = key[i*4+j];
			key_part = key_part << (8*j);
//			key_part = key_part << (24-(8*j));
			current += key_part;
//			current = current << (32-8*j);
		}
//		key_array[i] = reverse(current);
		key_array[i] = current;
	}
	key_in.word_0 = key_array[0];
	key_in.word_1 = key_array[1];
	key_in.word_2 = key_array[2];
	key_in.word_3 = key_array[3];

//	printf("\nOriginal key:\n0x");
	char bin_buffer[33];
	for(i=0; i<4; i++){
		int current_int = 0;
		for(j=0; j<4; j++){
			current_int = current_int << 8;
			current_int += key[i];
		}
		int2bin(current_int, bin_buffer, 32);
		bin_buffer[32] = '\0';
//		printf("%s", bin_buffer);
	}

//	printf("\nFabric key:\n0x");
	for(i=0; i<4; i++){
		int2bin(key_array[i], bin_buffer, 32);
		bin_buffer[32] = '\0';
//		printf("%s", bin_buffer);
	}
	begin = clock();
	XAes_Start(aes_device);

	XAes_Set_key_in_V(aes_device, key_in);
//	writeKey(key);
	XAes_Set_sourceAddress(aes_device, source);
//	writeSourceAddress(source);
	XAes_Set_destinationAddress(aes_device, dest);
//	writeDestinationAddress(dest);
	XAes_Set_length_r(aes_device, data_length);
//	writeLength(data_length);
//	int current_data_length = XAes_Get_length_r(aes_device);
//	printf("Current data length: %i\n", current_data_length);
	
	XAes_Set_sourceAddress_vld(aes_device);
//	writeLengthValid();
	XAes_Set_key_in_V_vld(aes_device);
//	writeDestinationAddressValid();
	XAes_Set_destinationAddress_vld(aes_device);
//	writeSourceAddressValid();
	XAes_Set_length_r_vld(aes_device);
//	writeKeyValid();
//	XAes_Start(aes_device);

	printf("\nWaiting for fabric.");
	while(XAes_IsDone(aes_device) != 1){
//		printf(".");
//		finished = XAes_GetFinished(aes_device);
	}
	int finished = XAes_Get_return(aes_device);
//	XAes_SetFinishedAck(aes_device);
	end = clock();
	ticks = (double)(end - begin);
	seconds = (double)(end - begin)/CLOCKS_PER_SEC;
	printf ("It took %f clicks (%f seconds) in fabric for %i encryptions.\n",ticks,seconds, data_length);
	FILE *aes_fabric_log;
	aes_fabric_log = fopen("aes_fabric_results.csv", "a");
	fprintf(aes_fabric_log, "%f,%i\n", seconds, data_length);
	fclose(aes_fabric_log);
//	printf("\nDone waiting for fabric\n");
//	for(i = 0; i<16; i++){
//		printf("%02x", ((char*)shared_system_mem->ptr)[16+i]);
//	}
//	printf("\n");
//	shared_system_mem = getSharedMemoryArea(source, length);
//	printf("\nNo segfault");
//	if(shared_system_mem == NULL){
//		printf("Error getting shared system memory pointer");
//		return -1;
//	}
	printf("\nWaiting for checking...");
//	int dummy = 0;
//	for(i=0; i<data_length*100; i++){
//		printf(".");
//		for(j=0; j<1000; j++){
//			dummy += i*j;
//		}
//	}
	printf("\n");
	int incorrectCount = 0;
//	printf("\nChar fabric\t|\tChar openssl\n");
//	printf("\nOpenssl:\n");
	printf("\nChecking");
	ticks = clock();
	for(i=0; i<data_length; i++){
//		printf("0x");
		for(j=0; j<16; j++){
			char openssl = encrypted_data_openssl[i*16 + j];
			int2bin(openssl, bin_buffer, 32);
//			printf("%02x",openssl);
			char fabric = ((char*)shared_system_mem->ptr)[i*16 + (15- j) + destOffset];
//			printf("\n%02x\t\t|\t%02x", fabric, openssl);
			if(openssl != fabric){
				printf("\nChar at index %i is not encrypted correctly. It is %02x in openssl, %02x in fabric", i*16+j, openssl, fabric);
				incorrectCount++;
			} 
		}
//		printf(" ");
//		printf("\n-------------------------------------");
	}
	ticks = clock() - ticks;
	printf ("\nIt took %f clicks (%f seconds) to check.\n",(float)ticks,((float)ticks)/CLOCKS_PER_SEC);
//	printf("\nFabric:\n");
//	for(i=0; i<data_length; i++){
//		printf("0x");
//		for(j=0; j<16; j++){
//			char fabric = ((char*)shared_system_mem->ptr)[i*16 + (15-j) + destOffset];
//			int2bin(fabric, bin_buffer, 32);
//			printf("%02x", fabric);
//		}
//		printf(" ");
//	}


//	printf("\n");
	printf("\nNum incorrect: %i\n", incorrectCount);
	cleanupSharedMemoryPointer(shared_system_mem);
	XAes_Release(aes_device);
//	free(aes_device);
//	free(key_in);
//	signalReadFinished();
	return 0;
}

