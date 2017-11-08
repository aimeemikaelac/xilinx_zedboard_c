#include <stdlib.h>
#include <sys/random.h>
#include <time.h>
#include "stdio.h"
#include "ed25519.h"
#include "user_mmap_driver.h"

#define SHARED_BUFFER 0xA0040000
#define RESET_CONTROL 0xB0000000
#define CONTROL_OFFSET 0x280
#define TRIALS 50
#define DATA_SIZE 0x100
#define CONTROL_OFFSET 0
#define PUBLIC_OFFSET 0x20
#define PRIVATE_OFFSET 0x40
#define SIGNATURE_OFFSET 0x80
#define DATA_OFFSET 0xC0


int main(){
  unsigned char seed[32], public_key[32], private_key[64], data[DATA_SIZE];
  unsigned char microblaze_signature[64], arm_signature[64];
  volatile unsigned char *shared_buffer, *control;
  shared_memory shared_buffer_mem;
  int i, trial_counter, matched;
  clock_t arm_start, arm_end, microblaze_start, microblaze_end;
  double arm_elapsed, microblaze_elapsed;
  //Use system() to call python programming script. As this program has to
  //be run as root anyways, this should be fine.
  //sudo ~/root_of_trust/operational_os/program_memory.py --bin ~/ecdsa_test.bin --base_address 0xA0000000
  system("/usr/bin/python /home/michael/root_of_trust/operational_os/program_memory.py --bin /home/michael/ecdsa_test.bin --base_address 0xA0000000");
  //reset system
  writeValueToAddress(1, RESET_CONTROL);
  //Set up shared memory
  shared_buffer_mem = getSharedMemoryArea(RESET_CONTROL, 0x2000);
  shared_buffer = shared_buffer_mem->ptr;
  control = shared_buffer + DATA_SIZE + CONTROL_OFFSET;
  //print out CSV header
  printf("ARM_SIGNATURE,ARM_TIME,MICROBLAZE_SIGNATURE,MICROBLAZE_TIME,DATA_SIZE,CORRECT\n");
  //Run several signatures
  for(trial_counter=0; trial_counter<TRIALS; trial_counter++){
    if(getrandom(seed, 32, GRND_NONBLOCK) < 0){
      fprintf(stderr, "Error getting random data. urandom may not be initialized.\n");
      return -1;
    }
    ed25519_create_keypair(public_key, private_key, seed);
    srand(((unsigned int *)(seed))[0]);
    for(i=0; i<DATA_SIZE; i++){
      data[i] = (unsigned char)rand();
    }
    arm_start = clock();
    ed25519_sign(arm_signature, data, public_key, private_key);
    arm_end = clock();
    //Copy public key
    for(i=0; i<32; i++){
      shared_buffer[PUBLIC_OFFSET + i] = public_key[i];
    }
    //Copy private key
    for(i=0; i<64; i++){
      shared_buffer[PRIVATE_OFFSET + i] = private_key[i];
    }
    //Copy data
    for(i=0; i<DATA_SIZE; i++){
      shared_buffer[DATA_OFFSET + i] = data[i];
    }
    //Clear last finished flag
    control[4] = 0;
    microblaze_start = clock();
    control[0] = 0xFF;
    while(control == 0){
      asm("");
      __asm__("");
    }
    microblaze_end = clock();
    //calculate result
    arm_elapsed = ((double)(arm_end - arm_start))/CLOCKS_PER_SEC;
    microblaze_elapsed = ((double)(microblaze_end - microblaze_start))/CLOCKS_PER_SEC;
    //copy in microblaze signature
    matched = 1;
    for(i=0; i<64; i++){
      microblaze_signature[i] = shared_buffer[SIGNATURE_OFFSET + i];
      if(arm_signature[i] != microblaze_signature[i]){
        matched = 0;
      }
    }
    //print result
    for(i=0; i<64; i++){
      printf("%02x", arm_signature[i]);
    }
    printf(",%f,", arm_elapsed);
    for(i=0; i<64; i++){
      printf("%02x", microblaze_signature[i]);
    }
    printf(",%f,%i,%i\n", microblaze_elapsed, DATA_SIZE, matched);
  }
}
