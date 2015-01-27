#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MEMSCANNER_BASE_ADDRESS "0x43C00000"
#define MEMSCANNER_AP_START_ENABLE_OFFSET 0x14
#define MEMSCANNER_AP_STATUS_OFFSET 0x1C

#define CONTROLLER_BASE_ADDRESS "0x43C10000"
#define CONTROLLER_START_ADDRESS_OFFSET 0x10
#define CONTROLLER_READ_LENGTH_OFFSET 0x18
#define CONTROLLER_ITERATIONS_OFFSET 0x20
#define CONTROLLER_ENABLED_OFFSET 0x28

//TODO: refactor this and write function, as setup and unmap is the same in both
//get the value at memory address gpio_addr in system address
//map using GPIO and /dev/map. program must be run as root
int getValueAtAddress(unsigned gpio_addr, int* value){
	unsigned page_addr, page_offset;
	//get page size from system
	unsigned page_size=sysconf(_SC_PAGESIZE);
	int fd;
	void * ptr;

	if (gpio_addr == 0) {
		printf("GPIO physical address is required.\n");
		return -1;
	}

	/* Open /dev/mem file */
	fd = open ("/dev/mem", O_RDWR);
	if (fd < 1) {
		perror("Error opening /dev/mem in getValueAtAddress: ");
		return -1;
	}

	/* mmap the device into memory */
	page_addr = (gpio_addr & (~(page_size-1)));
	page_offset = gpio_addr - page_addr;
	ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);


	/* Read value from the device register */
	*value = *((int *)(ptr + page_offset));
//	printf("gpio dev-mem test: input: %08x\n", *value);

	munmap(ptr, page_size);

	close(fd);

	return 0;
}

//write value to address in system memory map using gpio. program
//must still be run as root
int writeValueToAddress(int value, unsigned gpio_addr){
	unsigned page_addr, page_offset;
	//get page size from system
	unsigned page_size=sysconf(_SC_PAGESIZE);
	int fd;
	void * ptr;

	if (gpio_addr == 0) {
		printf("GPIO physical address is required.\n");
		return -1;
	}

	/* Open /dev/mem file */
	fd = open ("/dev/mem", O_RDWR);
	if (fd < 1) {
		perror("Error opening /dev/mem in getValueAtAddress: ");
		return -1;
	}

	/* mmap the device into memory */
	page_addr = (gpio_addr & (~(page_size-1)));
	page_offset = gpio_addr - page_addr;
	ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);

	/* Write value to the device register */
	*((unsigned *)(ptr + page_offset)) = value;
//	*((unsigned *)(ptr + page_offset + 8)) = value;

	munmap(ptr, page_size);

	close(fd);

	return 0;
}

unsigned getMemscannerBaseAddress(){
	return strtoul(MEMSCANNER_BASE_ADDRESS, NULL, 0);
}

unsigned getControllerBaseAddress(){
	return strtoul(CONTROLLER_BASE_ADDRESS, NULL, 0);
}

int getMemscannerOutput(){
	int output;
	getValueAtAddress(getMemscannerBaseAddress() + 0x14, &output);
	return output;
}

int getMemscannerMemoryValue(){
	int output;
	getValueAtAddress(getMemscannerBaseAddress() + 0x1c, &output);
	return output;
}

int getMemscannerCounterValue(){
	int output;
	getValueAtAddress(getMemscannerBaseAddress() + 0x24, &output);
	return output;
}

void setControllerStartAddress(unsigned startAddr){
	writeValueToAddress(startAddr, getControllerBaseAddress() + CONTROLLER_START_ADDRESS_OFFSET);
}

void setControllerReadLength(int length){
	writeValueToAddress(length, getControllerBaseAddress() + CONTROLLER_READ_LENGTH_OFFSET);
}

void setControllerIterations(int iterations){
	writeValueToAddress(iterations, getControllerBaseAddress() + CONTROLLER_ITERATIONS_OFFSET);
}

void setControllerEnabled(){
	writeValueToAddress(1, getControllerBaseAddress() + CONTROLLER_ENABLED_OFFSET);
}

void setControllerDisabled(){
	writeValueToAddress(0, getControllerBaseAddress() + CONTROLLER_ENABLED_OFFSET);
}

void setControllerApStart(){
	writeValueToAddress(1, getControllerBaseAddress());
}

int getControllerDmaControl(){
	int output;
	getValueAtAddress(getControllerBaseAddress() + 0x30, &output);
	return output;
}

int getControllerDmaStatus(){
	int output;
	getValueAtAddress(getControllerBaseAddress() + 0x38, &output);
	return output;
}

int main(void){
//	printf("Running memscanner on first 1000 word of system memory");
	unsigned startAddr = 0x10000000;
	setControllerApStart();
	setControllerStartAddress(startAddr);
	setControllerReadLength(4);
	setControllerIterations(1000);
	setControllerEnabled();
	//disabled so that is does not start again after it is finished?
	setControllerDisabled();
	int lastCounter = 0;
	int currentCounter = getMemscannerCounterValue();
	while(lastCounter < 1000){
		printf("Current counter: %d\n", getMemscannerCounterValue());
		printf("\nDMA control register value: %08x", getControllerDmaControl());
		printf("\nDMA status register value: %08x", getControllerDmaStatus());
		//spin while counter is the same
		while(lastCounter == currentCounter){}
		currentCounter = getMemscannerCounterValue();
		printf("\n---------\nGot output: %d\nMemory value as hex: %08x\nFor counter number: %d", getMemscannerOutput(), getMemscannerMemoryValue(), currentCounter);
		printf("\nDMA control register value: %08x", getControllerDmaControl());
		printf("\nDMA status register value: %08x", getControllerDmaStatus());
		printf("\n---------\n");
	}

	return 0;
}
