#include "user_mmap_driver.h"
#include "aes_fpga.h"
#include <openssl/rand.h>
#include <openssl/evp.h>
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

void checkFunction(char* encrypted_data_openssl, shared_memory shared_system_mem, int bytesToCheck, unsigned destOffset, unsigned hardwareOffset){
	int i, j;
	clock_t ticks;
	printf("\nWaiting for checking...");
//	int dummy = 0;
//	for(i=0; i<data_length*100; i++){
//		printf(".");
//		for(j=0; j<1000; j++){
//			dummy += i*j;
//		}
//	}
	printf("\n");
	char bin_buffer[33];
	int incorrectCount = 0;
//	printf("\nChar fabric\t|\tChar openssl\n");
//	printf("\nOpenssl:\n");
	printf("\nChecking");
	ticks = clock();
	for(i=0; i<bytesToCheck; i++){
//		printf("0x");
//		for(j=0; j<16; j++){
		char openssl = encrypted_data_openssl[i];
		int2bin(openssl, bin_buffer, 32);
//			printf("%02x",openssl);
			//char fabric = ((char*)shared_system_mem->ptr)[i*16 + (15- j) + destOffset];
		char fabric = ((char*)shared_system_mem->ptr)[hardwareOffset + i + destOffset];
//			printf("\n%02x\t\t|\t%02x", fabric, openssl);
		if(openssl != fabric){
			printf("\nChar at index %i is not encrypted correctly. It is %02x in openssl, %02x in fabric", i, openssl, fabric);
			incorrectCount++;
		} 
	}
//		printf(" ");
//		printf("\n-------------------------------------");
//	}
	ticks = clock() - ticks;
	printf ("\nIt took %f clicks (%f seconds) to check.\n",(double)ticks,((double)ticks)/CLOCKS_PER_SEC);
	printf("\nNum incorrect: %i\n", incorrectCount);

}

int main(int argc, char** argv){
	int i, j;
	unsigned char data_to_encrypt[]  = {0x01, 0x4B, 0xAF, 0x22, 0x78, 0xA6, 0x9D, 0x33, 0x1D, 0x51, 0x80, 0x10, 0x36, 0x43, 0xE9, 0x9A, '\0'};
//	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
	unsigned char key[]              = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
	unsigned char data_to_encrypt2[] = {0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, '\0'};
	unsigned char default_iv[]         = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\0'};
	int data_length;
	if(argc == 2){
		data_length = atoi(argv[1]);
	} else{
		data_length = 100;
	}
	printf("Data length: %i\n", data_length);
	unsigned char data_to_encrypt3[16*data_length];
	unsigned char encrypted_data_openssl[16*data_length+16];

	srand (time(NULL));

	int randStart = rand();
	for(i=0; i<data_length; i++){
		for(j=0; j<16; j++){
			data_to_encrypt3[16*i + j] = i*16+j;//+randStart;
		}
	}

//	AES_KEY aes_key;
//	AES_set_encrypt_key(key, 128, &aes_key);

	unsigned char* data_pointer = data_to_encrypt3;
	unsigned char* encrypted_dest = encrypted_data_openssl;

	clock_t begin, end;
	EVP_CIPHER_CTX ctx;
	EVP_EncryptInit(&ctx, EVP_aes_128_ecb(), key, default_iv);

	int num;

	printf("\nStart testing ECB mode for OpenSSL and hardware");
	printf("\n--------------------------------------------------------------------");

	begin = clock();
	EVP_EncryptUpdate(&ctx, encrypted_dest, &num, data_pointer, 16*data_length);
	EVP_EncryptFinal_ex(&ctx, encrypted_dest+16*data_length, &num);
//	for(i=0; i<data_length; i++){
//		AES_encrypt(data_pointer, encrypted_dest, &aes_key);	
//		data_pointer = data_pointer + 16;
//		encrypted_dest = encrypted_dest + 16;
//	}
	end = clock();
	double ticks = (double)(end - begin);
	double seconds =(double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in openssl.\n",ticks,seconds);
	FILE *openssl_fabric_log;
	openssl_fabric_log = fopen("aes_openssl_results.csv", "a");
	fprintf(openssl_fabric_log, "%f,%i\n", seconds, data_length);
	fclose(openssl_fabric_log);
	unsigned source = SHARED_MEM_BASE+5;
	unsigned length = SHARED_MEM_LENGTH-5;
	shared_memory shared_system_mem = getUioMemoryArea("/dev/uio1",0x80000);//getSharedMemoryArea(source, length);//getUioMemoryArea("/dev/uio1", length);//=
//	shared_memory shared_system_mem = getSharedMemoryArea(SHARED_MEM_BASE, 0x80000);
	if(shared_system_mem == NULL){
		printf("Error getting shared system memory pointer");
		return -1;
	}
	int destOffset = 16*data_length +16;
	int dest = source + destOffset;

	for(i=0; i<0x1000; i++){
		((char*)shared_system_mem->ptr)[i] = 0;
	}

	char bin_buffer[33];

	char *sourceData = (char*)(shared_system_mem->ptr)+5;
	char *destData = sourceData + destOffset;

	for(i=0; i<data_length; i++){
		for(j=0; j<16; j++){
			sourceData[i*16 + j] = data_to_encrypt3[i*16 + j];
		}
	}
	//-----------------------Switch to using the aes_fpga library code
	FPGA_AES *cipher = NULL;
	if((cipher = fpga_aes_new(key, 16, SHARED_MEM_BASE, "aes-qam", "axi-reset", default_iv, 16, 0)) == NULL){
		printf("\nCould not allocated cipher");
		return -1;
	}
	begin  = clock();
	Aes_encrypt_run(cipher, sourceData, data_length*16, destData, source, source + destOffset, 0);


	end = clock();
	ticks = (double)(end - begin);
	seconds = (double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in fabric for %i encryptions.\n",ticks,seconds, data_length);
	FILE *aes_fabric_log;
	aes_fabric_log = fopen("aes_fabric_results.csv", "a");
	fprintf(aes_fabric_log, "%f,%i\n", seconds, data_length);
	fclose(aes_fabric_log);

	checkFunction(encrypted_data_openssl, shared_system_mem, data_length*16, destOffset, 5);
	fpga_aes_free(cipher);
	
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
	printf("\nTesting CTR mode in OpenSSL and hardware");
	printf("\n-----------------------------------------------------------");

	for(i=0; i<data_length*16; i++){
		encrypted_dest[i] = 0;
		((char*)shared_system_mem->ptr)[i+destOffset] = 0;
	}

	begin = clock();
	EVP_EncryptInit(&ctx, EVP_aes_128_ctr(), key, default_iv);
	EVP_EncryptUpdate(&ctx, encrypted_dest, &num, data_pointer, 16*data_length);
	EVP_EncryptFinal_ex(&ctx, encrypted_dest+16*data_length, &num);
//	for(i=0; i<data_length; i++){
//		AES_encrypt(data_pointer, encrypted_dest, &aes_key);	
//		data_pointer = data_pointer + 16;
//		encrypted_dest = encrypted_dest + 16;
//	}
	end = clock();
	ticks = (double)(end - begin);
	seconds =(double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in openssl.\n",ticks,seconds);

	cipher = NULL;
	if((cipher = fpga_aes_new(key, 16, SHARED_MEM_BASE, "aes-qam", "axi-reset", default_iv, 16, 2)) == NULL){
		printf("\nCould not allocated cipher");
		return -1;
	}

	begin  = clock();
	//Aes_encrypt_run(cipher, sourceData, data_length*16, destData, SHARED_MEM_BASE, SHARED_MEM_BASE + destOffset, 0);
	Aes_encrypt_ctr_hw(cipher, sourceData, data_length*16, destData, source, source + destOffset);


	end = clock();

	ticks = (double)(end - begin);
	seconds = (double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in fabric for %i encryptions.\n",ticks,seconds, data_length);

/*	printf("\nDest offset: %i", destOffset);

	printf("\nOutput of ctr:\nFPGA: 0x");
	for(i=0; i<16; i++){
		printf("%02x", ((char*)shared_system_mem->ptr)[destOffset+i]);
	}
	printf("\nOpenssl: 0x");
	for(i=0; i<16; i++){
		printf("%02x", encrypted_dest[i]);
	}*/
	checkFunction(encrypted_data_openssl, shared_system_mem, data_length*16, destOffset, 5);
	fpga_aes_free(cipher);

	for(i=0; i<data_length*16; i++){
		encrypted_dest[i] = 0;
		((char*)shared_system_mem->ptr)[i+destOffset] = 0;
	}

/*	printf("\nInput to ctr:\nFPGA: 0x");
	for(i=0; i<16; i++){
		printf("%02x", ((char*)shared_system_mem->ptr)[i]);
	}
	printf("\n0x");
	for(i=16; i<32; i++){
		printf("%02x", ((char*)shared_system_mem->ptr)[i]);
	}
	printf("\nOpenssl: 0x");
	for(i=0; i<16; i++){
		printf("%02x", data_pointer[i]);
	}
	printf("\n0x");
	for(i=16; i<32; i++){
		printf("%02x", data_pointer[i]);
	}*/
/*
	printf("\nBeginning increment by 1 ECB test for OpenSSL");
	printf("\n-----------------------------------------------");

	begin=clock();
	
	EVP_EncryptInit(&ctx, EVP_aes_128_ecb(), key, default_iv);
	for(i=0; i<data_length*16; i++){
		EVP_EncryptUpdate(&ctx, encrypted_dest+i, &num, data_pointer+i, 1);
	}
	EVP_EncryptFinal_ex(&ctx, encrypted_dest+16*data_length, &num);

	end=clock();

	ticks = (double)(end - begin);
	seconds =(double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in openssl.\n",ticks,seconds);

	printf("\nBeginning increment by 1 ECB test for fabric");
	printf("\n-----------------------------------------------");

	begin = clock();
	for(i=0; i<data_length*16; i++){
		Aes_encrypt_run(cipher, sourceData+i, 1, destData+i, SHARED_MEM_BASE+i, SHARED_MEM_BASE + destOffset+i, 0);
	//	Aes_encrypt_ctr_hw(cipher, sourceData+i, 1, destData+i, SHARED_MEM_BASE+i, SHARED_MEM_BASE + destOffset + i);
	}

	end=clock();

	ticks = (double)(end - begin);
	seconds = (double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in fabric for %i encryptions.\n",ticks,seconds, data_length);
	checkFunction(encrypted_data_openssl, shared_system_mem, data_length, destOffset);
*/

	printf("\nBeginning increment by 1 CTR test for OpenSSL");
	printf("\n-----------------------------------------------");

	begin=clock();
	
	EVP_EncryptInit(&ctx, EVP_aes_128_ctr(), key, default_iv);
	for(i=0; i<data_length*16; i++){
/*		printf("\nCurrent evp iv: 0x");
		for(j=0; j<16; j++){
			printf("%02x", ctx.iv[j]);
		}
		printf("\nCurrent evp partial block: 0x");
		for(j=0; j<16; j++){
			printf("%02x", ctx.buf[j]);
		}*/
		EVP_EncryptUpdate(&ctx, encrypted_dest+i, &num, data_pointer+i, 1);
	}
	/*
	printf("\nOpenSSL ctr first 32 bytes of output: 0x");
	for(i=0; i<32; i++){
		if(i==16){
			printf(" 0x");
		}
		printf("%02x", encrypted_dest[i]);
	}*/

	EVP_EncryptFinal_ex(&ctx, encrypted_dest+16*data_length, &num);

	end=clock();

//	printf("\nFinal evp iv: 0x");
//	for(i=0; i<16; i++){
//		printf("%02x", ctx.iv[i]);
//	}
	ticks = (double)(end - begin);
	seconds =(double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in openssl.\n",ticks,seconds);

	printf("\nBeginning increment by 1 CTR test for fabric");
	printf("\n-----------------------------------------------");

	cipher = NULL;
	if((cipher = fpga_aes_new(key, 16, SHARED_MEM_BASE, "aes-qam", "axi-reset", default_iv, 16, 2)) == NULL){
		printf("\nCould not allocated cipher");
		return -1;
	}

	begin = clock();
	for(i=0; i<data_length*16; i++){
	/*	printf("\nCurrent value of output: 0x");
		for(j=0; j<16; j++){
			printf("%02x",destData[(i/16)*16+j]);
		}*/
//		Aes_encrypt_run(cipher, sourceData+i, 1, destData+i, SHARED_MEM_BASE+i, SHARED_MEM_BASE + destOffset+i, 0);
		Aes_encrypt_ctr_hw(cipher, sourceData+i, 1, destData+i, source+i, source + destOffset + i);
	}

	end=clock();

	ticks = (double)(end - begin);
	seconds = (double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in fabric for %i encryptions.\n",ticks,seconds, data_length);
	checkFunction(encrypted_data_openssl, shared_system_mem, data_length*16, destOffset, 5);



	printf("\nBeginning increment by 5 CTR test for OpenSSL");
	printf("\n-----------------------------------------------");

	begin=clock();
	
	EVP_EncryptInit(&ctx, EVP_aes_128_ctr(), key, default_iv);
	EVP_EncryptUpdate(&ctx, encrypted_dest, &num, data_pointer, (data_length-1)*16+5);
//	printf("\nCurrent evp partial block: 0x");
//	for(j=0; j<16; j++){
//		printf("%02x", ctx.buf[j]);
//	}

	EVP_EncryptFinal_ex(&ctx, encrypted_dest+16*data_length, &num);

	end=clock();

//	printf("\nFinal evp iv: 0x");
//	for(i=0; i<16; i++){
//		printf("%02x", ctx.iv[i]);
//	}

	ticks = (double)(end - begin);
	seconds =(double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in openssl.\n",ticks,seconds);

	printf("\nBeginning increment by 5 CTR test for fabric");
	printf("\n-----------------------------------------------");

	cipher = NULL;
	if((cipher = fpga_aes_new(key, 16, SHARED_MEM_BASE, "aes-qam", "axi-reset", default_iv, 16, 2)) == NULL){
		printf("\nCould not allocated cipher");
		return -1;
	}

	begin = clock();
//	for(i=0; i<data_length*16; i++){
//		Aes_encrypt_run(cipher, sourceData+i, 1, destData+i, SHARED_MEM_BASE+i, SHARED_MEM_BASE + destOffset+i, 0);
	Aes_encrypt_ctr_hw(cipher, sourceData, (data_length-1)*16+5, destData, source, source + destOffset);
//	}

	end=clock();

	ticks = (double)(end - begin);
	seconds = (double)(end - begin)/CLOCKS_PER_SEC;
	printf ("\nIt took %f clicks (%f seconds) in fabric for %i encryptions.\n",ticks,seconds, data_length);
	checkFunction(encrypted_data_openssl, shared_system_mem, (data_length-1)*16+5, destOffset, 5);




//	printf("\n");
	cleanupSharedMemoryPointer(shared_system_mem);
	fpga_aes_free(cipher);
//	XAes_Release(aes_device);
//	free(aes_device);
//	free(key_in);
//	signalReadFinished();
	return 0;
}

