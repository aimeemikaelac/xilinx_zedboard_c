#ifndef AES_FPGA_H_
#define AES_FPGA_H_

#include <stdio.h>
#include <string.h>
#include "user_mmap_driver.h"
#include "xaes.h"
#include "xreset_axi.h"

struct FPGA_AES;
typedef struct FPGA_AES FPGA_AES;

int aes_encrypt(FPGA_AES *cipher, size_t len, unsigned src_addr, unsigned dst_addr);

//need to follow convention: *to, *from, len, for simplicity
//int Aes_encrypt_memcpy(FPGA_AES *cipher, const char *input, size_t len, char *output);
int Aes_encrypt_memcpy(FPGA_AES *cipher, char *output, const char *input, size_t len);

//int Aes_encrypt_cbc_memcpy(FPGA_AES *cipher, const char *input, size_t len, char *output, char* iv);
int Aes_encrypt_cbc_memcpy(FPGA_AES *cipher, char* iv, char* output, const char *input, size_t len);

//int Aes_encrypt_cbc_memmgr(FPGA_AES *cipher, const char *input, size_t len, char *output, char *iv);
int Aes_encrypt_cbc_memmgr(FPGA_AES *cipher, char* iv, char* output, const char *input, size_t len);

//int Aes_encrypt_memmgr(FPGA_AES *cipher, const char *input, size_t len, char *output);
int Aes_encrypt_memmgr(FPGA_AES *cipher, char* output, const char *input, size_t len);

FPGA_AES *fpga_aes_new(const char *key, size_t key_len, unsigned shared_mem_base, char *device_name, char *rst_device);

void fpga_aes_free(FPGA_AES *cipher);

#endif