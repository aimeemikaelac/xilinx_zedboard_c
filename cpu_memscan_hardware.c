#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MEMSCANNER_BASE_ADDRESS "0x43C00000"
#define MEMSCANNER_AP_START_ENABLE_OFFSET 0x14
#define MEMSCANNER_AP_STATUS_OFFSET 0x1C

//#define CONTROLLER_BASE_ADDRESS "0x43C10000"
//#define CONTROLLER_START_ADDRESS_OFFSET 0x10
//#define CONTROLLER_READ_LENGTH_OFFSET 0x18
//#define CONTROLLER_ITERATIONS_OFFSET 0x20
//#define CONTROLLER_ENABLED_OFFSET 0x28

#define GPIO_0_ADDRESS 0x41200000
#define GPIO_1_ADDRESS 0x41210000
#define GPIO_2_ADDRESS 0x41220000
#define GPIO_3_ADDRESS 0x41230000
#define GPIO_PORT_1 0x00
#define GPIO_PORT_2 0x08

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
	writeValueToAddress(startAddr, GPIO_0_ADDRESS);
}

void setControllerReadLength(int length){
	writeValueToAddress(length, GPIO_0_ADDRESS + GPIO_PORT_2);
}

void setControllerIterations(int iterations){
	writeValueToAddress(iterations, GPIO_1_ADDRESS);
}

void setControllerEnabled(){
	writeValueToAddress(1, GPIO_1_ADDRESS + GPIO_PORT_2);
}

void setControllerDisabled(){
	writeValueToAddress(0, GPIO_1_ADDRESS + GPIO_PORT_2);
}

int getControllerDmaControl(){
	int output;
	getValueAtAddress(GPIO_2_ADDRESS, &output);
	return output;
}

int getControllerDmaStatus(){
	int output;
	getValueAtAddress(GPIO_2_ADDRESS + GPIO_PORT_2, &output);
	return output;
}

int getMemoryAddress(){
	int output;
	getValueAtAddress(GPIO_3_ADDRESS, &output);
	return output;
}

int main(void){
//	printf("Running memscanner on first 1000 word of system memory");
	unsigned startAddr = 0x10000000;
	int iterations = 100000;
	setControllerStartAddress(startAddr);
	setControllerReadLength(4);
	setControllerIterations(iterations);
	setControllerEnabled();
	unsigned lastCounter = getMemscannerMemoryValue();
	unsigned currentAddress = getMemoryAddress();
	int currentCounter = getMemscannerCounterValue();
	int dummy = 0;
	while(lastCounter < iterations && currentAddress < (startAddr + iterations * 4)){
		currentCounter = getMemscannerCounterValue();
		currentAddress = getMemoryAddress();
		printf("Current counter: %d\n", currentCounter);
		printf("\nDMA control register value: %08x", getControllerDmaControl());
		printf("\nDMA status register value: %08x", getControllerDmaStatus());
		printf("\nCurrent address: %08x", currentAddress);
		printf("\nTest");
		//spin while counter is the same
//		while(currentCounter < 1000 && lastCounter == currentCounter){
//			printf("\nTest");
//			dummy++;
//		}
		printf("\n---------\nGot output: %d\nMemory value as hex: %08x\nFor counter number: %d", getMemscannerOutput(), getMemscannerMemoryValue(), currentCounter);
		printf("\nDMA control register value: %08x", getControllerDmaControl());
		printf("\nDMA status register value: %08x", getControllerDmaStatus());
		printf("\n---------\n");
		lastCounter = currentCounter;
	}

	setControllerDisabled();
	printf("\nExiting\n");
	return 0;
}
