#include "user_mmap_driver.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
  long i;
  FILE *f = fopen("microblaze_hello_world.elf", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);  //same as rewind(f);

  char *file_data = malloc(fsize + 1);
  fread(file_data, fsize, 1, f);
  fclose(f);

  file_data[fsize] = 0;
  printf("file size: %i\n", fsize);

  shared_memory dest_mem = getSharedMemoryArea(0xB0002050, fsize);
  char *data_ptr = (char*)dest_mem->ptr;
  for(i=0; i<fsize; i++){
    data_ptr[i] = file_data[i];
  }

  cleanupSharedMemoryPointer(dest_mem);
}
