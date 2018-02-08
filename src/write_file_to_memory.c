#include "user_mmap_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

int main(int argc, char** argv){
  long i, address = 0;
  char *filename = NULL;
  unsigned int *file_buffer = NULL;

  while((c = getopt(argc, argv, "g:f")) != -1) {
		switch(c) {
		case 'a':
			address=strtoul(optarg,NULL, 0);
			break;
		case 'f':
			filename=optarg;
			break;
		default:
			printf("invalid option: %c\n", (char)c);
			usage();
			return -1;
		}

	}

  if(filename == NULL){
    printf("A filename is required\n");
    return -1;
  }

  if(address == 0){
    printf("A valid address is required\n");
    return -1;
  }

  int fp = open(filename, O_RDONLY);
  if(fp < 0){
    printf("Invalid file: %s\n", filename);
    return -1;
  }
  long fsize = get_size_by_fd(fp);

  printf("Writing %s of size %i to %i\n", filename, fsize, address);

  file_buffer = (unsigned int*)mmap(0, file_size, PROT_WRITE, MAP_READ, fp, 0);
  if(file_buffer == MAP_FAILED){
    printf("Could not map file\n");
    return -1;
  }

  for(i=0; i<file_size/sizeof(uint32_t); i++){
    writeValueToAddress(file_buffer[i], address + i);
  }

  munmap(file_buffer);
  return 0;
}
