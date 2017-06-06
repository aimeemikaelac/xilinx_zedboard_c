#include "user_mmap_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "crypto_sign.h"

#define MICROBLAZE_MEM_BASE 0xA0000000

unsigned char sk_ref[64]  =
    {0x7f,0x91,0x53,0x07,0xee,0x01,0xf3,0xfa,0x77,0xa4,0xff,0xb0,0xe8,
    0xf2,0x95,0x7b,0xe2,0x0d,0x46,0xea,0x5c,0x55,0x63,0x84,0x16,0xff,
    0x2e,0x26,0x52,0xc9,0x88,0xd1,0x9d,0x7c,0xc8,0x34,0x8f,0x23,0x5c,
    0xb5,0x96,0xec,0xf1,0x1e,0x35,0xe2,0xd5,0x6f,0x99,0xdf,0xae,0x10,
    0x6a,0x47,0x8f,0x0f,0x4f,0xe6,0x2a,0x13,0x7c,0x06,0x07,0xdf};

void usage(void)
{
    printf("*argv[0] -n <NUMBER_OF_BLOCKS>\n");
    printf("    -n <NUMBER_OF_BLOCKS>   Number of 128-byte blocks\n");
    return;
}

int main(int argc, char* argv[]){
  int c, i, j;
  unsigned int num_blocks = 0, current;
  unsigned long long int signed_mesage_length = 0;
  unsigned char *data_ptr, *message = NULL, *signed_message = NULL;
  unsigned int *message_int;

  while((c = getopt(argc, argv, "n:h")) != -1){
    switch(c) {
      case 'n':
        num_blocks = strtoul(optarg, NULL, 0);
        break;
      case 'h':
        usage();
        return 0;
        break;
      default:
        printf("Invalid option: %c\n", (char)c);
        usage();
        return -1;
    }
  }

  if(num_blocks == 0){
    printf("Specify a non-zero number of 128-bit blocks\n");
    usage();
    return -1;
  }

  shared_memory microblaze_mem =
    getSharedMemoryArea(MICROBLAZE_MEM_BASE, num_blocks*128);

  data_ptr = (unsigned char*)microblaze_mem->ptr;

  if((message = malloc(num_blocks*128)) == NULL){
    printf("Could not allocate temporary message buf\n");
    return -1;
  }

  message_int = (unsigned int*)message;

  for(i=0; i<num_blocks; i++){
    for(j=0; j<128/4; j++){
      message_int[i*(128/4) + j] = ((unsigned int*)(data_ptr))[i*(128/4) + j];
    }
  }

/*  printf("Original message/microblaze memory contents:\n");
  for(i=0; i<num_blocks; i++){
    for(j=0; j<128/4; j++){
      printf("%08x", message_int[i*(128/4) + j]);
    }
  }
  printf("\n");*/

  if((signed_message = malloc(num_blocks*128 + 64)) == NULL){
    printf("Could not allocated signed message buf\n");
    free(message);
    return -1;
  }

  crypto_sign(signed_message,
              &signed_mesage_length,
              message,
              128*num_blocks,
              sk_ref);

  printf("Signed message out:\n");
  for(i=0; i<64/4; i++){
    printf("%08x", ((unsigned int*)(signed_message))[i]);
  }
  printf("\n");
  free(message);
  free(signed_message);
}
