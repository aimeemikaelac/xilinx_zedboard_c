#include "user_mmap_driver.h"
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "aes_fpga.h"
#include <time.h>
#include "memmgr.h"
#include "xtriple_aes.h"

char *int2bin(int a, char *buffer, int buf_size) {
        buffer += (buf_size - 1);
        int i;
        for (i = 31; i >= 0; i--) {
                *buffer-- = (a & 1) + '0';
                
                a >>= 1;
        }
        
        return buffer;
}

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

u32 packU32(char one, char two, char three, char four){
	u32 data = 0;
	data = (u32)one + (((u32)two) << 8) + (((u32)three) << 16) + (((u32)four) << 24);
	return data;
}

int triple_encrypt_fpga_ecb(unsigned char *plaintext, int plaintext_len,
		unsigned char *key, unsigned char *key1, unsigned char *key2,
		unsigned char *ciphertext){
	clock_t begin, end;

	memmgr_assert(plaintext);
	memmgr_assert(ciphertext);

	unsigned src  = lookupBufferPhysicalAddress(plaintext);
	unsigned dest = lookupBufferPhysicalAddress(ciphertext);

	XTriple_aes_Key_in_v key_in;
	key_in.word_0 = packU32(key[0], key[1], key[2], key[3]);
	key_in.word_1 = packU32(key[4], key[5], key[6], key[7]);
	key_in.word_2 = packU32(key[8], key[9], key[10], key[11]);
	key_in.word_3 = packU32(key[12], key[13], key[14], key[15]);

	XTriple_aes_Key_in1_v key_in1;
	key_in1.word_0 = packU32(key1[0], key1[1], key1[2], key1[3]);
	key_in1.word_1 = packU32(key1[4], key1[5], key1[6], key1[7]);
	key_in1.word_2 = packU32(key1[8], key1[9], key1[10], key1[11]);
	key_in1.word_3 = packU32(key1[12], key1[13], key1[14], key1[15]);

	XTriple_aes_Key_in2_v key_in2;
	key_in2.word_0 = packU32(key2[0], key2[1], key2[2], key2[3]);
	key_in2.word_1 = packU32(key2[4], key2[5], key2[6], key2[7]);
	key_in2.word_2 = packU32(key2[8], key2[9], key2[10], key2[11]);
	key_in2.word_3 = packU32(key2[12], key2[13], key2[14], key2[15]);

	XTriple_aes_Iv_v iv_in;
	iv_in.word_0 = 0;
	iv_in.word_1 = 0;
	iv_in.word_2 = 0;
	iv_in.word_3 = 0;

	XTriple_aes_Iv1_v iv_in1;
	iv_in1.word_0 = 0;
	iv_in1.word_1 = 0;
	iv_in1.word_2 = 0;
	iv_in1.word_3 = 0;

	XTriple_aes_Iv2_v iv_in2;
	iv_in2.word_0 = 0;
	iv_in2.word_1 = 0;
	iv_in2.word_2 = 0;
	iv_in2.word_3 = 0;

	XTriple_aes triple_aes;
	XTriple_aes_Initialize(&triple_aes, "triple-aes");

	XTriple_aes_Start(&triple_aes);

	XTriple_aes_Set_sourceAddress(&triple_aes, src);
	XTriple_aes_Set_key_in_V(&triple_aes, key_in);
	XTriple_aes_Set_key_in1_V(&triple_aes, key_in1);
	XTriple_aes_Set_key_in2_V(&triple_aes, key_in2);
	XTriple_aes_Set_iv_V(&triple_aes, iv_in);
	XTriple_aes_Set_iv1_V(&triple_aes, iv_in1);
	XTriple_aes_Set_iv2_V(&triple_aes, iv_in2);
	XTriple_aes_Set_destinationAddress(&triple_aes, dest);
	XTriple_aes_Set_numBytes(&triple_aes, plaintext_len);
	XTriple_aes_Set_mode(&triple_aes, 0);

	XTriple_aes_Set_sourceAddress_vld(&triple_aes);
	XTriple_aes_Set_key_in_V_vld(&triple_aes);
	XTriple_aes_Set_key_in1_V_vld(&triple_aes);
	XTriple_aes_Set_key_in2_V_vld(&triple_aes);
	XTriple_aes_Set_iv_V_vld(&triple_aes);
	XTriple_aes_Set_iv1_V_vld(&triple_aes);
	XTriple_aes_Set_iv2_V_vld(&triple_aes);
	XTriple_aes_Set_destinationAddress_vld(&triple_aes);
	XTriple_aes_Set_numBytes_vld(&triple_aes);
	XTriple_aes_Set_mode_vld(&triple_aes);

	begin = clock();

	printf("\nWaiting for FPGA");

	while(XTriple_aes_IsDone(&triple_aes) != 1){}

	end = clock();
	double ticks = (double)(end-begin);
	double seconds = ticks/CLOCKS_PER_SEC;
	printf("It took %f clicks (%f seconds) in FPGA.\n", ticks, seconds);
	
	XTriple_aes_Release(&triple_aes);
}


int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int ciphertext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the encryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, iv))
    handleErrors();

  /* Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}


int main(int argc, char** argv){
        int i, j;

	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);

        unsigned char data_to_encrypt[] = {0x01, 0x4B, 0xAF, 0x22, 0x78, 0xA6, 0x9D, 0x33, 0x1D, 0x51, 0x80, 0x10, 0x36, 0x43, 0xE9, 0x9A, '\0'};

        unsigned char key[] = {0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
        unsigned char key1[] = {0xE9, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
        unsigned char key2[] = {0xEA, 0xE9, 0xEA, 0xEB, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5, 0xF7, 0xF8, 0xF9, 0xFA, '\0'};
        unsigned char iv[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '\0'};

        unsigned char data_to_encrypt2[] = {0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, '\0'};
	
	int data_length;
        if(argc == 2){
                data_length = atoi(argv[1]);
        } else{
                data_length = 100;
        }
        printf("Data length: %i\n", data_length);
        unsigned char data_to_encrypt3[16*data_length];
	memset(data_to_encrypt3, 0, 16*data_length);
        unsigned char encrypted_data_openssl[16*data_length*2];
	memset(encrypted_data_openssl, 0, 16*data_length);
//	unsigned char encrypted_data_fabric[16*data_length];
//	memset(encrypted_data_fabric, 0, 16*data_length);

	memmgr_init_shared_short();
	printf("\n");
	unsigned char *fpga_src  = memmgr_alloc(16*data_length);
	unsigned char *fpga_dest = memmgr_alloc(16*data_length);

        srand (time(NULL));

        int randStart = rand();
        for(i=0; i<data_length; i++){
                for(j=0; j<16; j++){
                        data_to_encrypt3[16*i + j] = i*j+j;//+randStart;
                }
        }


        unsigned char* data_pointer = data_to_encrypt3;
        unsigned char* encrypted_dest = encrypted_data_openssl;
//	memset(encrypted_dest, 16*data_length, 0);

        clock_t begin, end;
        begin = clock();
	encrypt(data_pointer, 16*data_length, key, iv, encrypted_dest);
	encrypt(encrypted_dest, 16*data_length, key1, iv, encrypted_dest);
	encrypt(encrypted_dest, 16*data_length, key2, iv, encrypted_dest);
	end = clock();
	double ticks = (double)(end - begin);
        double seconds =(double)(end - begin)/CLOCKS_PER_SEC;
        printf ("It took %f clicks (%f seconds) in openssl.\n",ticks,seconds);
//	FPGA_AES *cipher = fpga_aes_new(key, 16, 0x1f410000, "qam", "axi-reset");
        data_pointer = data_to_encrypt3;
//        encrypted_dest = encrypted_data_fabric;

        begin = clock();
//                Aes_encrypt_cbc_memcpy(cipher, iv, encrypted_dest, data_pointer, 16*data_length);    
	triple_encrypt_fpga_ecb(fpga_src, 16*data_length, key, key1, key2, fpga_dest);
	end = clock();
	ticks = (double)(end - begin);
        seconds =(double)(end - begin)/CLOCKS_PER_SEC;
        printf ("\nIt took %f clicks (%f seconds) in fabric.\n",ticks,seconds);

	printf("\nChecking");
        ticks = clock();

	char bin_buffer[33];
	bin_buffer[32] = '\0';
	int incorrectCount = 0;

	int k = 0;
	for(i=0; i<1000000; i++){
		k+=5;
	}
	printf("\nk is: %i", k);
	
	if(data_length > 5){

	        for(i=0; i<data_length; i++){
        	        for(j=0; j<16; j++){
	                        char openssl = encrypted_data_openssl[i*16 + j];
                	        int2bin(openssl, bin_buffer, 32);
        	                char fabric = fpga_dest[i*16+j];
	                        if(openssl != fabric){
	                                printf("\nChar at index %i is not encrypted correctly. It is %02x in openssl, %02x in fabric", i*16+j, openssl, fabric);
	                                incorrectCount++;
	                        } 
	                }
	        }
	        ticks = clock() - ticks;
	        printf ("\nIt took %f clicks (%f seconds) to check.\n",(float)ticks,((float)ticks)/CLOCKS_PER_SEC);
	} else{
		printf("\nInputs:\n");
		for(i=0; i<data_length; i++){
			printf(" 0x");
			for(j=0; j<16; j++){
				printf("%02x", data_to_encrypt3[i*16+j]);
			}
		}
		printf("\nOpenSSL results:\n");
		for(i=0; i<data_length; i++){
			printf(" 0x");
			for(j=0; j<16; j++){
				printf("%02x", encrypted_data_openssl[i*16+j]);
			}
		}
		printf("\nFabric results:\n");
		for(i=0; i<data_length; i++){
			printf(" 0x");
			for(j=0; j<16; j++){
				printf("%02x", fpga_dest[i*16+j]);
			}
		}

	}
	printf("\n");

	EVP_cleanup();
	ERR_free_strings();
	memmgr_free(fpga_src);
	memmgr_free(fpga_dest);
	return 0;
}
