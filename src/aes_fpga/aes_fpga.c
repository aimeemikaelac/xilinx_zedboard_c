#include "aes_fpga.h"
#include "memmgr.h"
#include <openssl/evp.h>
#include <pthread.h>
/*
struct FPGA_AES{
	const char* key;
	int key_length_bits;
	char* device;
	char* rst_device;
	unsigned shared_mem_base;
};*/

//byte reverse every 16 bytes of a char buffer 
//length is the total length of char buffer
//there will be length/16 buffer reversals
void byteReverseBuffer16(char* buffer, int length){
	int i, j, tmp, iterLen, bufferIndex;
//	iterLen = length/16;
//	need to use ceiling of division:
	iterLen = length/16 + (length % 16 != 0);
	for(i=0; i<iterLen; i++){
		bufferIndex = 16*i;
		for(j=0; j<8; j++){
	//		((char*)mem->ptr)[i*16+j] = input[i*16 + (15-j)];
			tmp = buffer[bufferIndex + j];
			buffer[bufferIndex + j] = buffer[bufferIndex + 15-j];
			buffer[bufferIndex + 15-j] = tmp;
		}
	}
}

void byteReverseBuffer8(char* buffer, int length){
	int i, j, tmp, iterLen, bufferIndex;
//	need to use ceiling of division:
	iterLen = length/8 + (length % 8 != 0);
	for(i=0; i<iterLen; i++){
		bufferIndex = 8*i;
		for(j=0; j<4; j++){
			tmp = buffer[bufferIndex + j];
			buffer[bufferIndex + j] = buffer[bufferIndex + 7-j];
			buffer[bufferIndex + 7-j] = tmp;
		}
	}

}

void byteReverseBuffer4(char* buffer, int length){
	int i, j, tmp, iterLen, bufferIndex;
//	need to use ceiling of division:
	iterLen = length/4 + (length % 4 != 0);
	for(i=0; i<iterLen; i++){
		bufferIndex = 4*i;
		for(j=0; j<2; j++){
			tmp = buffer[bufferIndex + j];
			buffer[bufferIndex + j] = buffer[bufferIndex + 3-j];
			buffer[bufferIndex + 3-j] = tmp;
		}
	}

}

//data must be aligned to 16 bytes and zero-padded if needed
//len is provided as number of bytes to encrypt
int aes_encrypt(FPGA_AES *cipher, size_t len, unsigned src_addr, unsigned dst_addr, int mode){
	int i, j;
	XReset_axi reset_axi;
	if(XReset_axi_Initialize(&reset_axi, cipher->rst_device) != XST_SUCCESS){
//		__android_log_print(ANDROID_LOG_DEBUG, "ndktest_jni", "\nCould not initialize axi reset device");
		printf("\nCould not initialize axi reset device: %s", cipher->rst_device);
		return -1;
	}
//	printf("\nResetting");
	XReset_axi_SetIn_reset(&reset_axi, 1);
//	printf("\nReset asserted");
	XReset_axi_SetIn_reset(&reset_axi, 0);
//	printf("\nReset deasserted");
	XReset_axi_Release(&reset_axi);

	XAes aes_device;

	if(XAes_Initialize(&aes_device, cipher->device) != XST_SUCCESS){

//	         __android_log_print(ANDROID_LOG_DEBUG, "ndktest_jni", "\nCould not initialize aes device");
	         printf("\nCould not initialize aes device: %s", cipher->device);
	         return -2;
	}
	
	XAes_Key_in_v key_in;
	XAes_Iv_v iv_in;
	u32 key_array[4];
	u32 iv_array[4];
/*	for(i=0; i<4; i++){
		u32 current = 0;
	        for(j=0; j<4; j++){
		        int key_part = cipher->key[15-i*4-j];
	        	key_part = key_part << (8*j);
		        current += key_part;
		}
		key_array[i] = current;
	}*/
	//FPGA should be able to perform key and iv reversal. Just fill up the struct
	for(i=0; i<4; i++){
		int curIndex = i*4;
		u32 current_key = cipher->key[curIndex] + (cipher->key[curIndex+1]*0x100) + (cipher->key[curIndex+2]*0x10000) + (cipher->key[curIndex+3]*0x1000000);
		u32 current_iv = cipher->iv[curIndex] + (cipher->iv[curIndex+1]*0x100) + (cipher->iv[curIndex+2]*0x10000) + (cipher->iv[curIndex+3]*0x1000000);
		key_array[i] = current_key;
		iv_array[i] = current_iv;
	}


	key_in.word_0 = key_array[0];
	key_in.word_1 = key_array[1];
	key_in.word_2 = key_array[2];
	key_in.word_3 = key_array[3];

	iv_in.word_0 = iv_array[0];
	iv_in.word_1 = iv_array[1];
	iv_in.word_2 = iv_array[2];
	iv_in.word_3 = iv_array[3];

	unsigned source = src_addr;
	unsigned dest = dst_addr;
	//TODO: take ceil of len/16
//	unsigned data_length = len/16 + (len % 16 != 0);//len/16;

//	We will only perform encryptions of the block size in the FPGA for now
	unsigned data_length = (len/16)*16;
//	unsigned data_length = len/16;
	
	printf("\nNumber of FPGA iterations: %i", data_length);

//	printf("\nStarting AES");	

	XAes_Start(&aes_device);

	XAes_Set_key_in_V(&aes_device, key_in);

	XAes_Set_sourceAddress(&aes_device, source);

	XAes_Set_destinationAddress(&aes_device, dest);

	XAes_Set_numBytes(&aes_device, data_length);

	XAes_Set_iv_V(&aes_device, iv_in);

	XAes_Set_mode(&aes_device, mode);

//	XAes_Set_length_r(&aes_device, data_length);

	XAes_Set_sourceAddress_vld(&aes_device);

	XAes_Set_key_in_V_vld(&aes_device);

	XAes_Set_destinationAddress_vld(&aes_device);

	XAes_Set_numBytes_vld(&aes_device);

	XAes_Set_iv_V_vld(&aes_device);

	XAes_Set_mode_vld(&aes_device);

//	XAes_Set_length_r_vld(&aes_device);

//	printf("\nWaiting for fabric");

	int count = 0;

	while(XAes_IsDone(&aes_device) != 1){
		count++;
	}
	
//	printf("\nIterations of while loop while waiting: %i", count);

	int finished = XAes_Get_return(&aes_device);

	XAes_Release(&aes_device);

//	printf("\nFinished");

	return finished;
}

//assume that the iv is 16 bits, as is supposed to be
int Aes_encrypt_cbc_memcpy(FPGA_AES *cipher, char* iv, char* output, const char *input, size_t len){
	Aes_encrypt_cbc_run(cipher, input, len, output, iv, 1);
}

int Aes_encrypt_cbc_memmgr(FPGA_AES *cipher, char* iv, char* output, const char *input, size_t len){
	Aes_encrypt_cbc_run(cipher, input, len, output, iv, 0);
}

//mode: choose between use memcpy or memmgr aes functions
//	0: memmgr (default)
//	1: memcpy
int Aes_encrypt_cbc_run(FPGA_AES *cipher, const char *input, size_t len, char *output, char *iv, int mode){
	int i, j;	
	char *current_iv = iv;
	char current_data[16];
	char *current_out = output;
	int num_encryptions = len/16;
	for(i=0; i<num_encryptions; i++){
		for(j=0; j<16; j++){
			current_data[j] = current_iv[j] ^ input[i*16 + j];
		}
		if(mode == 1){
			if(Aes_encrypt_memcpy(cipher, current_out, current_data, 16) != 0){
				return -1;
			}
		} else{
			if(Aes_encrypt_memmgr(cipher, current_out, current_data, 16) != 0){
				return -1;
			}
		}
		current_iv = current_out;
		current_out = current_out + 16;
	}
	return 0;
}

//TODO: assuming iv[0] is least significant bit
int incrementIv(char* iv, int iv_length){
	int i;
	unsigned int current;
	//start at beginning
//	for(i=0; i<iv_length; i++){
	for(i=iv_length-1; i>=0; i--){
		//increment this bit
		current = (unsigned int)iv[i];
//		current = current + 1;
		current = current + 1;
		iv[i] = (unsigned char)current;
		//if the incrementation does not rollover, i.e. 0xff + 1 == 0x00,
		//then we can a stop, else increment next bit. if is for examle, 0x00ffff,
		//then the first iteration will result in 0x00ff00, the second will result
		//in 0x000000, the third will result in 0x010000
		if(current < 0xff){
			break;
		}
	}
}

int addIv(char* iv, int iv_length, unsigned int num){
	int i;
	char carry =0;
	for(i=0; i<iv_length; i++){
		char low = iv[i];
		char currentNum = (char)(num >> 8*i);
		iv[i] = low + currentNum + carry;
		if(iv[i] < low){
			carry = 1;
		} else{
			carry = 0;
		}
	}
	return 0;
}

int printIv(char* iv, int iv_length){
	int i;
	int current = 0;
	char* currentChar = (char*)(&current);
	printf("\n0x");
	for(i=iv_length-1; i>=0; i--){
//	for(i=0; i<iv_length; i++){
		printf("%02x", iv[i]);
		current+=(int)iv[i];
	}
//	printf("\nInt value: %i", current);
//	printf("\nRepresentation:\n0x");
//	for(i=0; i<sizeof(int)/sizeof(char); i++){
//		printf("%02x", currentChar[i]);
//	}
//	printf("\nSizeof(int): %i, Sizeof(long): %i", sizeof(int), sizeof(long));
}

struct ctr_thread_data{
	int thread_id;
	FPGA_AES *cipher;
	char* input;
	size_t len;
	char* output;
	unsigned sourceAddress;
	unsigned destAddress;
	int mode;
};

void* pthread_Ctr_hw_ex(void* ctr_thread_data_arg){
	struct thread_data* = (struct ctr_thread_data*)(ctr_thread_data_arg);
	Aes_encrypt_run(thread_data->cipher, thread_data->input, thread_data->len, thread_data->output, thread_data->sourceAddress, thread_data->destAddress, thread_data->mode);

}

int Aes_encrypt_ctr_hw(FPGA_AES *cipher, char *input, size_t len, char *output, unsigned src, unsigned dest){
	int i, num, rc = 0;
//	int numEncryptions = len/16 + (len%16 != 0);
	int numBytesExtra = len%16;
	int numFullSegments = len/16;
	pthread_t aes_thread;
	pthread_attr_t attr;
	void* status;
	//initialize pthread variables
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	//Encrypt all of the full 16 byte segments
	//do not want to use the fpga if we do not have any full segments
	if(numFullSegments > 0){
		//TODO: have the FPGA return the counter, so we don't have to
		//calculate it
		//Aes_encrypt_run(cipher, input numFullSegments, output, src, dest, 2);
		struct str_thread_data aes_args;
		aes_args.thread_id = 0;
		aes_args.cipher = cipher;
		aes_args.input = input;
		aes_args.len = numFullSegments;
		aes_args.output = output;
		aes_args.sourceAddress = src;
		aes_args.destAddress = dest;
		aes_args.mode = 2;
		rc = pthread_create(&aes_thread, &attr, pthread_Ctr_hw_ex, &aes_args);
		if(rc){
			printf("\nError creating aes pthread. Return code is %d", rc);
			abort();
		}
	}
	//Encrypt the last segment by calculating the counter
	//TODO: farm this part to a thread
	if(numBytesExtra > 0){
		char local_iv[16];
		char temp[16];
		for(i=0; i<16; i++){
			local_iv[i] = cipher->iv[i];
		}
		//calculate IV
	//	for(i=0; i<numFullSegments; i++){
	//		incrementIv(local_iv, 16);
	//	}
		addIv(temp, 16, numFullSegments);
		//call openssl aes ctr and output to the correct place, reading from the 
		//last segment that is not full
		EVP_CIPHER_CTX ctx;
		EVP_EncryptInit(&ctx, EVP_aes_128_ctr(), cipher->key, local_iv);
		EVP_EncryptUpdate(&ctx, output+(numFullSegments*16), &num, input+(numFullSegments*16), numBytesExtra);
		EVP_EncryptFinal_ex(&ctx, temp, &num);
	}
	if(numFullSegments > 0){
		rc = pthread_join(aes_thread, &status);
		if(rc){
			printf("\nError encuntered when joining aes pthread. Return code is: %d", rc);
			abort();
		}
	}

	return 0;
}


//implements ctr in software on top of AES in hardware
//assumes that the buffers are accessible by the FPGA -> e.g. in correct
//memory address to match the src and dest addresses
//TODO: for ctr mode, the input does not need to be in the FPGA for now,
//nut a precomputed count value could be, else this will be slow
//TODO: implement ctr in hardware so that this is not a problem
//TODO: need to determine if iv[0] or iv[iv_length] is the least significant
//bit for incrementing
int Aes_encrypt_ctr_run(FPGA_AES *cipher, char *input, size_t len, char* output, unsigned src, unsigned dest){
	int i, j;
	char temp[cipher->iv_length];
	unsigned char iv[cipher->iv_length];
	int numEncryptions = len/16;
	char* current = iv;
	char* current_in = input;
	char* current_out = output;
	unsigned current_src = src;
	unsigned current_dest = dest;
	//store a local copy of iv
	for(i=0; i<cipher->iv_length; i++){
		iv[i] = cipher->iv[i];
	}
//	char tempOut[cipher->iv_length];
//	printf("\nNumber of CTR encryptions: %i", numEncryptions);
//	printf("\nCipher iv length: %i", cipher->iv_length);
	for(i=0; i<numEncryptions; i++){
		//TODO: assuming for now that iv_length is the same as a single 
		//aes encryption. for now, 16 bytes
		
//		printf("\nCurrent source: 0x%08x", current_src);
//		printf("\nCurrent dest: 0x%08x", current_dest);
//		printf("\nCurrent input: %p", current_in);
//		printf("\nCurrent output: %p", current_out);
		printf("\nCurrent iv:");
		printIv(iv, cipher->iv_length);
		
		//for ctr, encrypt the iv, then xor with the pt. then increment the 
		//iv and xor the next ouput with the next pt set
		//need to put counter into the current input segment and
		//store the pt
		for(j=0; j<16; j++){
			temp[j] = current_in[j];
			current_in[j] = iv[j];
		}
		Aes_encrypt_run(cipher, current_in, 16, current_out, current_src, current_dest, 0);
		for(j=0; j<16; j++){
			current_out[j] = current_out[j]^temp[j];
		}
		//need to change the input back to its original state
		for(j=0; j<16; j++){
			current_in[j] = temp[j];
		}
		//increment by the number of bytes encrypted
	//	for(j=0; j<16; j++){
			incrementIv(iv, cipher->iv_length);
	//	}
		current_in = current_in + 16;
		current_out = current_out + 16;
		current_src = current_src + 16;
		current_dest = current_dest + 16;
	}
	return 0;
}

//memcpy to the shared memory from the source and memcpy back to the dest
//this function does not need src and dest addresses, as it can allocate
//wherever it needs to in the shared mem area
//NOTE: for the current AES fpga implementation, be sure to put the input
//in byte-reversed AND pull the output out byte-reversed. Cannot use
//system memcpy for that
int Aes_encrypt_memcpy(FPGA_AES *cipher, char* output, const char *input, size_t len){
	shared_memory mem = NULL;
	int i, j;
	int iterLen = len/16;
	if((mem = getSharedMemoryArea(cipher->shared_mem_base, len*3)) == NULL){
		printf("\nCould not get shared memory area");
		return -1;
	}
	unsigned src = cipher->shared_mem_base;
	//printf("\nSource: 0x%02x", src);
	unsigned dest = src+len*sizeof(char);
	//printf("\nDest: 0x%02x", dest);

	memcpy((void*)(mem->ptr), (const void*)(input), len);

	Aes_encrypt_run(cipher, input, len, output, src, dest, 0);

	memcpy((void*)(output), (void*)(mem->ptr) + len*sizeof(char), len);

	cleanupSharedMemoryPointer(mem);
	return 0;
}

int Aes_encrypt_memmgr(FPGA_AES *cipher, char* output, const char *input, size_t len){
	void* in;
	void* out;
	unsigned src, dest;
	int iterLen;
	char tmp;
	in = (void*)input;
	out = (void*)output;
	//check that the input was allocated by the 
	memmgr_assert((void*)input);
	memmgr_assert((void*)output);
	src = lookupBufferPhysicalAddress((void*)input);
	printf("\nLookup of source address: %08x", src);
	dest = lookupBufferPhysicalAddress((void*)output);
	printf("\nLookup of destination address: %08x", dest);

//	iterLen = len/16;
	//byte reverse the input	
//	for(i=0; i<iterLen; i++){
		//swap bytes at front with bytes at back, till we hit the middle
//		byteReverseBuffer16(input+i);
//	}
//	byteReverseBuffer16(input, len);


//	aes_encrypt(cipher, len, src, dest);

//	char *aes_out = ((char*)(mem->ptr)) + len;
//	char *output_rw = output;
	
//	memcpy((void*)(output), (const void*)(aes_out), len);
//	for(i=0; i<iterLen; i++){
	//	for(j=0; j<16; j++){
	//		output_rw[i*16+j] = aes_out[i*16 + (15-j)];
	//	}
//		byteReverseBuffer16(
//	}
//	byteReverseBuffer16(output, len);

	Aes_encrypt_run(cipher, input, len, output, src, dest, 0);

	//TODO: may need to byte-reverse the input again
	return 0;
}

//assume that the input is in the correct memory region now
//byte reverse the input pointer, call fpga and reverse output
int Aes_encrypt_run(FPGA_AES *cipher, const char *input, size_t len, char *output, unsigned src, unsigned dest, int mode){
	byteReverseBuffer16((char*)input, len);
//	byteReverseBuffer4((char*)input, len);
//	byteReverseBuffer8((char*)input, len);

	aes_encrypt(cipher, len, src, dest, mode);

//	byteReverseBuffer16(output, len);
//	byteReverseBuffer4(output, len);
//	byteReverseBuffer8(output, len);
	return 0;
}


//create a new FPGA AES struct, with info on the shared memory region
FPGA_AES* fpga_aes_new(const char *key, size_t key_len, unsigned shared_mem_base, char* device_name, char* rst_device, char* iv, int iv_length){
	FPGA_AES *cipher = NULL;
	if((cipher=malloc(sizeof(FPGA_AES))) == NULL){
		return NULL;
	}
	cipher->key=key;
	cipher->key_length_bits = key_len;
	cipher->device = device_name;
	cipher->rst_device = rst_device;
	cipher->shared_mem_base = shared_mem_base;
	cipher->iv = iv;
	cipher->iv_length = iv_length;
	return cipher;
}


//free an fpga aes struct
void fpga_aes_free(FPGA_AES *cipher){
	free(cipher);
}
