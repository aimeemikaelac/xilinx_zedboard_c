#include "aes_fpga.h"
#include "memmgr.h"
//#include <openssl/evp.h>
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
//	printf("\nResetting");
	XReset_axi_SetIn_reset(cipher->reset_axi, 1);
//	printf("\nReset asserted");
	XReset_axi_SetIn_reset(cipher->reset_axi, 0);
//	printf("\nReset deasserted");

	unsigned source = src_addr;
	unsigned dest = dst_addr;
	//TODO: take ceil of len/16
//	unsigned data_length = len/16 + (len % 16 != 0);//len/16;

//	We will only perform encryptions of the block size in the FPGA for now
	unsigned data_length = (len/16)*16;
//	unsigned data_length = len/16;
	
//	printf("\nNumber of FPGA iterations: %i", data_length);

//	printf("\nStarting AES");	
	u32 iv_array[4];
	for(i=0; i<4; i++){
		int curIndex = i*4;
		u32 current_iv = cipher->iv[curIndex] + (cipher->iv[curIndex+1]*0x100) + (cipher->iv[curIndex+2]*0x10000) + (cipher->iv[curIndex+3]*0x1000000);
		iv_array[i] = current_iv;
	}


	cipher->iv_in.word_0 = iv_array[0];
	cipher->iv_in.word_1 = iv_array[1];
	cipher->iv_in.word_2 = iv_array[2];
	cipher->iv_in.word_3 = iv_array[3];

	XAes_Start(cipher->aes_device);

	XAes_Set_key_in_V(cipher->aes_device, cipher->key_in);

	XAes_Set_sourceAddress(cipher->aes_device, source);

	XAes_Set_destinationAddress(cipher->aes_device, dest);

	XAes_Set_numBytes(cipher->aes_device, data_length);

	XAes_Set_iv_V(cipher->aes_device, cipher->iv_in);

//	XAes_Iv_v iv_temp = XAes_Get_iv_V(cipher->aes_device);
//	printf("\nCurrent iv in hardware:");
//	printf("\n0x%08x", iv_temp.word_0);
//	printf("\n0x%08x", iv_temp.word_1);
//	printf("\n0x%08x", iv_temp.word_2);
//	printf("\n0x%08x", iv_temp.word_3);

	XAes_Set_mode(cipher->aes_device, mode);

//	XAes_Set_length_r(&aes_device, data_length);

	XAes_Set_sourceAddress_vld(cipher->aes_device);

	XAes_Set_key_in_V_vld(cipher->aes_device);

	XAes_Set_destinationAddress_vld(cipher->aes_device);

	XAes_Set_numBytes_vld(cipher->aes_device);

	XAes_Set_iv_V_vld(cipher->aes_device);

	XAes_Set_mode_vld(cipher->aes_device);

//	XAes_Set_length_r_vld(&aes_device);

//	printf("\nWaiting for fabric");

	int count = 0;

	while(XAes_IsDone(cipher->aes_device) != 1){
		count++;
	}
	
//	printf("\nIterations of while loop while waiting: %i", count);

	int finished = XAes_Get_return(cipher->aes_device);

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
//	printIv(iv, 16);
	for(i=0; i<iv_length; i++){
//		printIv(iv, iv_length);
		char low = iv[i];
//		printf("\n%02x", (unsigned char)(num));
		char currentNum = (char)(num >> 8*i);
//		printf("\n%02x", (unsigned char)(currentNum));
		iv[i] = low + currentNum + carry;
		if(iv[i] < low){
			carry = 1;
		} else{
			carry = 0;
		}
	}
	return 0;
}

int addIvOpenssl(char* iv, int iv_length, unsigned int num){
	int i;
	char carry =0;
//	printIv(iv, 16);
	for(i=iv_length-1; i>=0; i--){
//		printIv(iv, iv_length);
		char low = iv[i];
//		printf("\n%02x", (unsigned char)(num));
		char currentNum = (char)(num >> 8*(iv_length-i-1));
//		printf("\n%02x", (unsigned char)(currentNum));
		iv[i] = low + currentNum + carry;
		if(iv[i] < low){
			carry = 1;
		} else{
			carry = 0;
		}
	}
	return 0;
}

int addIvChar(char* iv, int iv_length, char* num){
	int i;
	char carry =0;
//	printIv(iv, 16);
	for(i=0; i<iv_length; i++){
//		printIv(iv, iv_length);
		char low = iv[i];
		char current = num[i];
//		printf("\n%02x", (unsigned char)(num));
//		char currentNum = (char)(num >> 8*i);
//		printf("\n%02x", (unsigned char)(currentNum));
		iv[i] = low + current + carry;
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
//	for(i=iv_length-1; i>=0; i--){
	for(i=0; i<iv_length; i++){
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

/*struct ctr_thread_data{
	int thread_id;
	FPGA_AES *cipher;
	char* input;
	size_t len;
	char* output;
	unsigned sourceAddress;
	unsigned destAddress;
	int mode;
};*/

void* pthread_Ctr_hw_ex(void* ctr_thread_data_arg){
	ctr_hw_thread_data* thread_data_arg = (ctr_hw_thread_data*)(ctr_thread_data_arg);
//	printf("\nRunning ctr hw thread");
//	printf("\nHw len: %i", thread_data_arg->len);
//	byteReverseBuffer16(thread_data_arg->input, thread_data_arg->len);
	Aes_encrypt_run(thread_data_arg->cipher, thread_data_arg->input, thread_data_arg->len, thread_data_arg->output, thread_data_arg->sourceAddress, thread_data_arg->destAddress, thread_data_arg->mode);
//	byteReverseBuffer16(thread_data_arg->output, thread_data_arg->len);
	pthread_exit(NULL);

}

void* pthread_Ctr_sw_ex(void* ctr_thread_data_arg){
	ctr_sw_thread_data* thread_data_arg = (ctr_sw_thread_data*)(ctr_thread_data_arg);
	char incrementVal[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}; 
	int num, i;
	char local_iv[16];
	char temp[16];
	for(i=0; i<16; i++){
		local_iv[i] = thread_data_arg->cipher->iv[i];
	}
	//calculate IV
//	for(i=0; i<numFullSegments; i++){
//		incrementIv(local_iv, 16);
//	}
	int offset = thread_data_arg->offset;
	int numBytes = thread_data_arg->numBytes;
	char* output = thread_data_arg->output;
	char* input = thread_data_arg->input;
//	printIv(thread_data_arg->cipher->ctx->iv, 16);
//	printf("\nCalling add iv");
//	for(i=0; i<thread_data_arg->increment; i++){
//		addIvChar(local_iv, thread_data_arg->cipher->iv_length, incrementVal);//thread_data_arg->increment);
//	}
//	printf("\nbytes processed: %i, num bytes: %i", thread_data_arg->cipher->bytesProcessed, numBytes);
	int bytesProcessed = thread_data_arg->cipher->bytesProcessed;
//	addIvOpenssl(local_iv, 16, (bytesProcessed/16) + (bytesProcessed % 16 != 0));
//	printIv(local_iv, 16);
	//call openssl aes ctr and output to the correct place, reading from the 
	//last segment that is not full
	EVP_CIPHER_CTX ctx;
	EVP_EncryptInit(&ctx, EVP_aes_128_ctr(), thread_data_arg->cipher->key, local_iv);
//	printf("\n2. Bytes processd: %i, Openssl iv:", thread_data_arg->cipher->bytesProcessed);
//	printIv(ctx.iv, 16);
	EVP_EncryptUpdate(&ctx, output+(offset), &num, input+(offset), numBytes);
//	printf("\nOffset: %i, numBytes: %i, input: %p, output: %p", offset, numBytes, input, output);
//	EVP_EncryptUpdate(&ctx, output+offset, &num, input+offset, numBytes);
//	printf("\n1. Bytes processd: %i, Openssl iv:", thread_data_arg->cipher->bytesProcessed);
//	printIv(thread_data_arg->cipher->ctx->iv, 16);
//	EVP_EncryptUpdate(thread_data_arg->cipher->ctx, thread_data_arg->output+(thread_data_arg->offset), &num, thread_data_arg->input+(thread_data_arg->offset), thread_data_arg->numBytes);
//	printf("\n2. Bytes processd: %i, Openssl iv:", thread_data_arg->cipher->bytesProcessed);
//	printIv(thread_data_arg->cipher->ctx->iv, 16);
	//printIv(thread_data_arg->cipher->iv, 16);
//	EVP_EncryptFinal_ex(thread_data_arg->cipher->ctx, temp, &num);
	pthread_exit(NULL);
}


int Aes_encrypt_ctr_hw(FPGA_AES *cipher, char *input, size_t len, char *output, unsigned src, unsigned dest){
	int i, num, rc = 0;
//	int numEncryptions = len/16 + (len%16 != 0);
//	printf("\nLength: %i", len);
//	printIv(cipher->iv, 16);
//	printf("\nNum bytes extra: %i, Num full segments: %i", numBytesExtra,numFullSegments);
//	pthread_t aes_thread;
	pthread_t openssl_thread;
	pthread_attr_t attr;
	void* status;

	int run_pthread = 0;
	//store the original pt into the cipher's buffer
	//this could segfault if the input buffer is read beyond,
	//so this must be accounted for
//	if(numBytesExtra > 0){
//		for(i=0; i<16; i++){
//			cipher->buf[i] = input[offset+i];
//		}
//	}
	cipher->bytesProcessed += len;
	int scrollIndex = 0;
	//if num > 0, then we are still in a partial block from last encryption
	//check if the source address < 16 + the last block address
	//	if so, then just scroll forward through the encrypted bytes in the buffer
	//	until we hit the 16 byte boundary, or the number of bytes to encrypt
	//		if we hit the 16 byte boundary,then reset num to 0
	//		and return, as we do not want to encrypt anything else
	if(cipher->currentBlockStart == 0){
		cipher->currentBlockStart = src;
	}
//	printf("\nOriginal output before scrolling: 0x");
//	for(i=0; i<16; i++){
//		printf("%02x", (output-(src-cipher->currentBlockStart))[i]);
//		printf("%02x", cipher->currentBlockStart[i]);
//	}
	int startingNewBlock = 0;
	
	//check if we are still in the valid area
	//or if we need to scroll forward to fill the last block
	if(cipher->num > 0 && (src < cipher->currentBlockStart + 16 && src > cipher->currentBlockStart || cipher->currentBlockStart + cipher->num + len == src)){
		printf("\nScrolling");
		//if for some reason we are not at the place we left off,
		//just put us in the correct place
		if(src != cipher->currentBlockStart + cipher->num){
//			printf("\nCorrecting num. Was: %i ", cipher->num);
			cipher->num = src - cipher->currentBlockStart;
//			printf("is now: %i", cipher->num);
		}

//		printf("\nNum is now: %i", cipher->num);
		
		while(scrollIndex < len && cipher->num<16){
			printf("\nReplacing %02x with %02x", output[scrollIndex], cipher->buf[cipher->num]);
			output[scrollIndex] = cipher->buf[cipher->num];
			scrollIndex++;
			cipher->num++;
//			cipher->bytesProcessed++;
		}
		//if we have finished with the partial block, reset the counter
		//also need to increment to iv
		if(cipher->num == 16){
			cipher->num = 0;
			cipher->currentBlockStart = cipher->currentBlockStart + 16;
			//incrementIv(cipher->iv, 16);
//			addIvOpenssl(cipher->iv, 16, 1);
//			addIv(cipher->iv, 16, 1);
	//		for(i=0; i<16; i++){
	//			cipher->iv[i] = 0xff;
	//		}
			startingNewBlock = 1;
		}
//		printf("\nScrolled by: %i", scrollIndex);
		//if we hve hit the length limit, then return
		if(scrollIndex >= len){
			return 0;
		}
	}
	//we are not in the correct place. need to reset num
	else if(cipher->num > 0){
//		printf("\nI think i am in the incorrect place.");
//		printf("\nCurrent block start: 0x%08x", cipher->currentBlockStart);
//		printf("\nCurrent num: %i", cipher->num);
//		printf("\nInput src address: 0x%08x", src);
		cipher->num = 0;
		cipher->currentBlockStart = src;
	}
	printf("\nCurrent cipher block start: 0x%08x", cipher->currentBlockStart);
//	printf("\nOriginal output mid: 0x");
//	for(i=0; i<16; i++){
//		printf("%02x", (output-(src-cipher->currentBlockStart))[i]);
//	}
	int currentLen = len - scrollIndex;//startingNewBlock == 1? scrollIndex : len - scrollIndex;
	int numBytesExtra = currentLen%16;
	int numFullSegments = currentLen/16;
	int numSegmentsTotal = numFullSegments + (numBytesExtra != 0);
	int offset = startingNewBlock? numFullSegments*16 : numFullSegments*16+scrollIndex;
	unsigned currentSrc = src+scrollIndex;
	unsigned newBlockStart = currentSrc + numFullSegments*16;
	unsigned currentDest = dest+scrollIndex;
	char* currentInput = input+scrollIndex;
	char* currentOutput = output+scrollIndex;
	char storage[16];
	//if there are numBytes extra, grab the bytes already in the final segment
	//and store them
	if(numBytesExtra > 0){
		printf("\nStoring bytes: 0x");
		for(i=0; i<16; i++){
			storage[i] = currentOutput[offset+i];
			printf("%02x", storage[i]);
		}
	}

	//increment the bytes starting at the scroll index
	//calculate the number of segments here based on the scroll index
//	printf("\nScroll index: %i", scrollIndex);
//	printf("\nNum bytes extra: %i", numBytesExtra);
//	printf("\nNum full segments: %i", numFullSegments);
	printf("\nCurrent src: 0x%08x, dest: 0x%08x", currentSrc, currentDest);
//	printf("\nnum segments total: %i", numSegmentsTotal);
	printf("\nOffset: %i", offset);
//	printf("\nOriginal input: %p, current: %p", input, currentInput);
//	printf("\nOriginal output: %p, current: %p", output, currentOutput);
//	printf("\nCurrent FPGA input: 0x");
//	for(i=0; i<16; i++){
//		printf("%02x", currentInput[i]);
//	}
	//need to increment the iv by the number of full segments that have been process
	//by this cipher
	
	Aes_encrypt_run(cipher, currentInput, numSegmentsTotal*16, currentOutput, currentSrc, currentDest, 2);
	addIvOpenssl(cipher->iv, 16, numSegmentsTotal);
		printf("\nOriginal output 1st full segment: 0x");
		for(i=0; i<16; i++){
			printf("%02x", currentOutput[i]);
		}

	//if there numBytes extra after scrolling, store the encrypted output into the buffer
	//put the original contents of the output back for what was not encrypted
	if(numBytesExtra > 0){
		printf("\nExtra bytes: %i", numBytesExtra);
		printf("\nCurrent cipher block start: 0x%08x", cipher->currentBlockStart);
		cipher->currentBlockStart = newBlockStart;//currentSrc;//+offset;
		printf("\nNew cipher block start: 0x%08x", cipher->currentBlockStart);
		printf("\nOriginal output: 0x");
		for(i=0; i<16; i++){
			printf("%02x", currentOutput[offset+i]);
		}
		for(i=0; i<16; i++){
			cipher->buf[i] = currentOutput[offset+i];
			if(i >= numBytesExtra){
				printf("\nExtra bytes. Restoring %02x from storage. Overwriting %02x", storage[i], currentOutput[offset+i]);
				currentOutput[offset+i] = storage[i];
			}
		}
//		printf("\nOriginal output end: 0x");
//		for(i=0; i<16; i++){
//			printf("%02x", currentOutput[offset+i]);
//		}
		cipher->num = numBytesExtra;
//		cipher->bytesProcessed+=numBytesExtra;
	}


	//Encrypt all of the full 16 byte segments
	//do not want to use the fpga if we do not have any full segments
//	if(numFullSegments > 0){
		//TODO: have the FPGA return the counter, so we don't have to
		//calculate it
		//Aes_encrypt_run(cipher, input numFullSegments, output, src, dest, 2);
/*		ctr_thread_data aes_args;
		aes_args.thread_id = 0;
		aes_args.cipher = cipher;
		aes_args.input = input;
		aes_args.len = numFullSegments*16;
		aes_args.output = output;
		printf("\nSource: 0x%08x", src);
		aes_args.sourceAddress = src;
		printf("\nDest: 0x%08x", dest);
		aes_args.destAddress = dest;
		aes_args.mode = 2;
		rc = pthread_create(&aes_thread, &attr, pthread_Ctr_hw_ex, &aes_args);
		if(rc){
			printf("\nError creating aes pthread. Return code is %d", rc);
			abort();
		}*/
//		Aes_encrypt_run(cipher, input, numFullSegments*16, output, src, dest, 2);
//	}
//	cipher->bytesProcessed += numFullSegments*16;
	//Encrypt the last segment by calculating the counter
	//TODO: farm this part to a thread
//	cipher->bytesProcessed += numBytesExtra;
//	if(numBytesExtra > 0){
//		run_pthread = 1;
		//initialize pthread variables
/*		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		ctr_sw_thread_data openssl_args;
		openssl_args.thread_id = 0;
		openssl_args.cipher = cipher;
		openssl_args.input = input;
		openssl_args.offset = numFullSegments*16;
		openssl_args.output = output;
		openssl_args.numBytes = numBytesExtra;
		openssl_args.increment = numFullSegments;

		rc = pthread_create(&openssl_thread, &attr, pthread_Ctr_sw_ex, &openssl_args);
		if(rc){
			printf("\nError creating openssl pthread. Return code is %d", rc);
			abort();
		}*/
//	}
//	if(run_pthread == 1){
//		rc = pthread_join(aes_thread, &status);
/*		rc = pthread_join(openssl_thread, &status);
		if(rc){
			//printf("\nError encountered when joining aes pthread. Return code is: %d", rc);
			printf("\nError encountered when joining openssl pthread. Return code is: %d", rc);
			abort();
		}*/
//	}


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
//	byteReverseBuffer16((char*)input, len);
//	byteReverseBuffer4((char*)input, len);
//	byteReverseBuffer8((char*)input, len);

	aes_encrypt(cipher, len, src, dest, mode);

//	byteReverseBuffer16(output, len);
//	byteReverseBuffer4(output, len);
//	byteReverseBuffer8(output, len);
	return 0;
}


//create a new FPGA AES struct, with info on the shared memory region
FPGA_AES* fpga_aes_new(const char *key, size_t key_len, unsigned shared_mem_base, char* device_name, char* rst_device, char* iv, int iv_length, int mode){
	int i;
	FPGA_AES *cipher = NULL;
	if((cipher=malloc(sizeof(FPGA_AES))) == NULL){
		return NULL;
	}
	cipher->key=key;
	cipher->key_length_bits = key_len;
	cipher->device = device_name;
	cipher->rst_device = rst_device;
	cipher->shared_mem_base = shared_mem_base;
	char* iv_local = NULL;
	iv_local = malloc(iv_length*sizeof(char));
	if(iv_local == NULL){
		printf("\nCould not alloc cipher iv");
		return NULL;
	}
	for(i=0; i<iv_length; i++){
		iv_local[i] = iv[i];
	}
	cipher->iv = iv_local;
	cipher->iv_length = iv_length;

	XReset_axi *reset_axi = NULL;
	reset_axi = malloc(sizeof(XReset_axi));

	if(reset_axi == NULL){
		printf("\nCould not malloc reset axi struct");
		free(cipher);
		free(iv_local);
		return NULL;
	}

	if(XReset_axi_Initialize(reset_axi, cipher->rst_device) != XST_SUCCESS){
//		__android_log_print(ANDROID_LOG_DEBUG, "ndktest_jni", "\nCould not initialize axi reset device");
		printf("\nCould not initialize axi reset device: %s", cipher->rst_device);
		free(cipher);
		free(reset_axi);
		free(iv_local);
		return NULL;
	}

	XAes *aes_device = NULL;
	aes_device = malloc(sizeof(XAes));

	if(aes_device == NULL){
		printf("\nCould not malloc aes device struct");
		free(cipher);
		XReset_axi_Release(reset_axi);
		free(reset_axi);
		free(iv_local);
		return NULL;
	}

	if(XAes_Initialize(aes_device, cipher->device) != XST_SUCCESS){
//	         __android_log_print(ANDROID_LOG_DEBUG, "ndktest_jni", "\nCould not initialize aes device");
	         printf("\nCould not initialize aes device: %s", cipher->device);
		 free(cipher);
		 XReset_axi_Release(reset_axi);
		 free(reset_axi);
		 free(aes_device);
		free(iv_local);
	         return NULL;
	}
	cipher->reset_axi = reset_axi;
	cipher->aes_device = aes_device;

	EVP_CIPHER_CTX *ctx = NULL;
	ctx = malloc(sizeof(EVP_CIPHER_CTX));
	if(ctx == NULL){
		printf("\nCould not malloc evp context");
		free(cipher);
		XReset_axi_Release(reset_axi);
		free(reset_axi);
		XAes_Release(aes_device);
		free(aes_device);
		free(iv_local);
		return NULL;
	}
	if(mode == 2){
		EVP_EncryptInit(ctx, EVP_aes_128_ctr(), key, iv);
	} else if(mode == 1){
		EVP_EncryptInit(ctx, EVP_aes_128_cbc(), key, iv);
	} else{
		EVP_EncryptInit(ctx, EVP_aes_128_ecb(), key, iv);
	}

	cipher->ctx = ctx;

	cipher->mode = mode;

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
//		u32 current_iv = cipher->iv[curIndex] + (cipher->iv[curIndex+1]*0x100) + (cipher->iv[curIndex+2]*0x10000) + (cipher->iv[curIndex+3]*0x1000000);
		key_array[i] = current_key;
//		iv_array[i] = current_iv;
	}


	key_in.word_0 = key_array[0];
	key_in.word_1 = key_array[1];
	key_in.word_2 = key_array[2];
	key_in.word_3 = key_array[3];

//	iv_in.word_0 = iv_array[0];
//	iv_in.word_1 = iv_array[1];
//	iv_in.word_2 = iv_array[2];
//	iv_in.word_3 = iv_array[3];

	cipher->key_in = key_in;
	cipher->iv_in = iv_in;

	cipher->bytesProcessed = 0;
	cipher->num=0;
	cipher->currentBlockStart = 0;

	return cipher;
}


//free an fpga aes struct
void fpga_aes_free(FPGA_AES *cipher){
	XReset_axi_Release(cipher->reset_axi);
	XAes_Release(cipher->aes_device);
	free(cipher->iv);
	free(cipher->ctx);
	free(cipher->reset_axi);
	free(cipher->aes_device);
	free(cipher);
	cipher = NULL;
}
