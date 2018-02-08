#include "user_mmap_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}


void usage(void)
{
	printf("*argv[0] -g <ADDRESS> -f <FILE>\n");
	printf("    -g <ADDR>   GPIO physical address\n");
	printf("    -f          File to progam\n");
	return;
}

int main(int argc, char** argv){
  int c;
  long i, address = 0;
  char *filename = NULL;
  unsigned int *file_buffer = NULL;

  while((c = getopt(argc, argv, "g:f")) != -1) {
		switch(c) {
		case 'g':
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

  printf("Writing %s of size %i to %li\n", filename, fsize, address);

  file_buffer = (unsigned int*)mmap(0, fsize, PROT_READ, MAP_SHARED, fp, 0);
  if(file_buffer == MAP_FAILED){
    printf("Could not map file\n");
    return -1;
  }

  for(i=0; i<fsize/sizeof(unsigned int); i++){
    writeValueToAddress(file_buffer[i], address + i);
  }

  munmap(file_buffer, fsize);
  return 0;
}
