#ifndef AES_FPGA_H_
#define AES_FPGA_H_

#include <stdio.h>
#include <string.h>
#include "user_mmap_driver.h"
#include "xaes.h"
#include "xreset_axi.h"

//struct FPGA_AES;

struct FPGA_AES{
        const char* key;
        int key_length_bits;
        char* device;
        char* rst_device;
        unsigned shared_mem_base;
	char* iv;
	int iv_length;
	XReset_axi* reset_axi;
	XAes* aes_device;
	XAes_Key_in_v key_in;
	XAes_Iv_v iv_in;
};

struct ctr_thread_data{
	int thread_id;
	struct FPGA_AES *cipher;
	char* input;
	size_t len;
	char* output;
	unsigned sourceAddress;
	unsigned destAddress;
	int mode;
};


typedef struct FPGA_AES FPGA_AES;
typedef struct ctr_thread_data ctr_thread_data;

void byteReverseBuffer8(char* buffer, int length);

void byteReverseBuffer16(char* buffer, int length);

int aes_encrypt(FPGA_AES *cipher, size_t len, unsigned src_addr, unsigned dst_addr, int mode);

//need to follow convention: *to, *from, len, for simplicity
//int Aes_encrypt_memcpy(FPGA_AES *cipher, const char *input, size_t len, char *output);
int Aes_encrypt_memcpy(FPGA_AES *cipher, char *output, const char *input, size_t len);

//int Aes_encrypt_cbc_memcpy(FPGA_AES *cipher, const char *input, size_t len, char *output, char* iv);
int Aes_encrypt_cbc_memcpy(FPGA_AES *cipher, char* iv, char* output, const char *input, size_t len);

//int Aes_encrypt_cbc_memmgr(FPGA_AES *cipher, const char *input, size_t len, char *output, char *iv);
int Aes_encrypt_cbc_memmgr(FPGA_AES *cipher, char* iv, char* output, const char *input, size_t len);

//int Aes_encrypt_memmgr(FPGA_AES *cipher, const char *input, size_t len, char *output);
int Aes_encrypt_memmgr(FPGA_AES *cipher, char* output, const char *input, size_t len);

int Aes_encrypt_run(FPGA_AES *cipher, const char *input, size_t len, char *output, unsigned src, unsigned dest, int mode);

int Aes_encrypt_ctr_run(FPGA_AES *cipher, char *input, size_t len, char* output, unsigned src, unsigned dest);

FPGA_AES *fpga_aes_new(const char *key, size_t key_len, unsigned shared_mem_base, char *device_name, char *rst_device, char* iv, int iv_length);

void fpga_aes_free(FPGA_AES *cipher);

#endif
