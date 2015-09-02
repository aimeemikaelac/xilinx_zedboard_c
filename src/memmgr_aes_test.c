#include "memmgr.h"
#include "user_mmmap_driver.h"
#include "aes_fpga.h"

int main(){
	int i;
	unsigned shared_size = 8*1024*1024;
	unsigned base_address = 0x1f41000;
	char* sharedUioDevice = "/dev/uio1";
	unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};

	AES_KEY aes_key;
	AES_set_encrypt_key(key, 128, &aes_key);


	shared_memory shared_mem = getUioMemoryArea(sharedUioDevice, shared_size);
	memmgr_init(shared_mem->ptr, shared_size, base_address);

	data1 = memmgr_alloc(1024);
	data2 = memmgr_alloc(1024);
	encrypted_dest = memmgr_alloc(1024);

	data[0]='\0';
	for(i=1023; i>0; i-= 35){
		strcat(data1, "Now is the time for all good onions", i);
	}

	FPGA_AES *cipher1 = NULL;
	if((cipher1 = fpga_aes_new(key, 16, base_address, sharedUioDevice, "axi-reset")) == NULL){
		printf("\nCould not allocate cipher1");
		abort();
	}

	Aes_encrypt_memmgr(cipher1, data2, data1, 512);
	AES_encrypt(data1, encrypted_dest, &aes_key);
	
	printf("\nFPGA\t|\tOpenSSL");
	for(i=0; i<16; i++){
		printf("\n0x%02x\t|\t0x%02x", data2[i]. encrypted_dest[i]);
	}

	memmgr_free(data1);
	memmgr_free(data2);
	memmgr_free(encrypted_dest);
	fpga_aes_free(cipher1);
}
