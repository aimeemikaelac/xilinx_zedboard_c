struct FPGA_AES{
	char* key;
	int key_length_bits;
	char** device;
	char** rst_device;
	unsigned shared_mem_base;
};

//data must be aligned to 16 bytes and zero-padded if needed
int aes_encrypt(FPGA_AES *cipher, shared_memory src, shared_mem dest, int len, unsigned src_addr, unsigned dst_addr){
	XReset_axi reset_axi;
	if(XReset_axi_Initialize(&reset_axi, cipher->rst_device) != XST_SUCCESS){
//		__android_log_print(ANDROID_LOG_DEBUG, "ndktest_jni", "\nCould not initialize axi reset device");
		printf("\nCould not initialize axi reset device: %s", cipher->rst_device);
		return -1;
	}
	XReset_axi_SetIn_reset(&reset_axi, 1);
	XReset_axi_SetIn_reset(&reset_axi, 0);
	XReset_axi_Release(&reset_axi);

	XAes aes_device;

	if(XAes_Initialize(&aes_device, cipher->device) != XST_SUCCESS){
//	         __android_log_print(ANDROID_LOG_DEBUG, "ndktest_jni", "\nCould not initialize aes device");
	         printf("\nCould not initialize aes device: %s", cipher->device
	         return -2;
	}
	
	XAes_Key_in_v key_in;
	u32 key_array[4];
	for(i=0; i<4; i++){
		u32 current = 0;
	        for(j=0; j<4; j++){
		        int key_part = cipher->key[15-i*4-j];
	        	key_part = key_part << (8*j);
		        current += key_part;
		}
		key_array[i] = current;
	}

	key_in.word_0 = key_array[0];
	key_in.word_1 = key_array[1];
	key_in.word_2 = key_array[2];
	key_in.word_3 = key_array[3];

	unsigned source = src_addr;
	unsigned dest = dst_addr;
	unsigned data_length = len/16;

	XAes_Start(&aes_device);

	XAes_Set_key_in_V(&aes_device, key_in);

	XAes_Set_sourceAddress(&aes_device, source);

	XAes_Set_destinationAddress(&aes_device, dest);

	XAes_Set_length_r(&aes_device, data_length);

	XAes_Set_sourceAddress_vld(&aes_device);

	XAes_Set_key_in_V_vld(&aes_device);

	XAes_Set_destinationAddress_vld(&aes_device);

	XAes_Set_length_r_vld(&aes_device);

	printf("\nWaiting for fabric");

	int finished = XAes_Get_return(aes_device);

	while(XAes_IsDone(&aes_device) != 1){}

	XAes_Release(&aes_device);

	return finished;
}

//memcpy to the shared memory from the source and memcpy back to the dest
//this function does not need src and dest addresses, as it can allocate
//wherever it needs to in the shared mem area
int Aes_encrypy_memcpy(){

}


//create a new FPGA AES struct, with info on the shared memory region
FPGA_AES* fpga_aes_new(){

}


//free an fpga aes struct
void fpga_aes_free(){

}
