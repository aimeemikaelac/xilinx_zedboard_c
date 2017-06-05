#include "xcrypto_sigh_hw.h"
#include "user_mmap_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MICROBLAZE_MEM_BASE 0xA0000000
#define CRYPTO_SIGN_BASE 0xB0010000

void usage(void)
{
	printf("*argv[0] -n <NUMBER_OF_BLOCKS>\n");
	printf("    -n <NUMBER_OF_BLOCKS>   Number of 128-byte blocks\n");
	return;
}

unsigned char sk[64] =
          {0x7f,0x91,0x53,0x07,0xee,0x01,0xf3,0xfa,0x77,0xa4,0xff,0xb0,
           0xe8,0xf2,0x95,0x7b,0xe2,0x0d,0x46,0xea,0x5c,0x55,0x63,0x84,
           0x16,0xff,0x2e,0x26,0x52,0xc9,0x88,0xd1,0x9d,0x7c,0xc8,0x34,
           0x8f,0x23,0x5c,0xb5,0x96,0xec,0xf1,0x1e,0x35,0xe2,0xd5,0x6f,
           0x99,0xdf,0xae,0x10,0x6a,0x47,0x8f,0x0f,0x4f,0xe6,0x2a,0x13,
           0x7c,0x06,0x07,0xdf};

int main(int argc, char* argv[]){
  int c, i;
  unsigned int num_blocks;
  unsigned char *data_ptr;
  unsigned int *control_register, *sk_reg, *blocks_reg, *sig_out;

  while((c = getopt(argc, argv, "n:h")) != -1){
    switch(c) {
      case 'n':
        num_blocks = strtoul(optarg, NULL, 0);
        break;
      case 'h':
        usage();
        break;
      default:
        printf("Invalid option: %c\n", (char)c);
        usage();
        return -1;
    }
  }

  shared_memory sign_device = getSharedMemoryArea(CRYPTO_SIGN_BASE, 0x1000);
  data_ptr = (unsigned char*)sign_device->ptr;
  control_register = (unsigned int*)data_ptr;
  sk_reg = (unsigned int*)(data_ptr + XCRYPTO_SIGN_AXILITES_ADDR_SK_BASE);
  blocks_reg =
    (unsigned int*)(data_ptr + XCRYPTO_SIGN_AXILITES_ADDR_NUM_BLOCKS_DATA);
  sig_out =
    (unsigned int*)(data_ptr + XCRYPTO_SIGN_AXILITES_ADDR_SIGNATURE_OUT_BASE);

  for(i=0; i<64/4; i++){
    sk_reg[i] = ()(unsigned int*)(sk))[i];
  }

  *blocks_reg = num_blocks;

  *control_register = 1;

  while((*control_register) & 0x2 == 0){
    __asm__("");
		asm("");
  }

  printf("Crypto sign done\n");

  printf("Signature out:\n");
  for(i=0; i<64/4; i++){
    printf("%08x", sig_out[i]);
  }
  printf("\n");

}
