#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

//control bits for the axi dma taken from LogiCORE IP AXI DMA v7.1, PG021 April 2, 2014 from Xilinx - the documentation for the AXI_DMA core

#define AXI_DMA_BASE_ADDRESS "0x40400000"
#define AXI_RESET_BIT 3
//the run/stop bit in the AXI_DMA control register
#define AXI_RS_BIT 1

#define MEMSCANNER_BASE_ADDRESS "0x43C00000"
#define MEMSCANNER_AP_START_ENABLE_OFFSET 0x14
#define MEMSCANNER_AP_STATUS_OFFSET 0x1C

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

unsigned getAxiDmaBaseAddress(){
	return strtoul(AXI_DMA_BASE_ADDRESS, NULL, 0);
}

unsigned getMemscannerBaseAddress(){
	return strtoul(MEMSCANNER_BASE_ADDRESS, NULL, 0);
}

void enableAxiMM2SControlBit(int bitIntValue){
	unsigned addr = getAxiDmaBaseAddress();
	printf("\nBase address: %08x\n", addr);
	int statusValue;
	getValueAtAddress(addr, &statusValue);
	printf("\nOriginal status value: %08x\n", statusValue);
	statusValue &= bitIntValue;
	printf("\nStatus value: %08x\n", statusValue);
	writeValueToAddress(statusValue, addr);
}

void disableAxiMM2SControlBit(int bitIntValue){
	unsigned addr = getAxiDmaBaseAddress();
	int statusValue;
	int notBitIntValue = ~bitIntValue;
	getValueAtAddress(addr, &statusValue);
	statusValue &= notBitIntValue;
	writeValueToAddress(statusValue, addr);
}

void enableResetAxiMM2SDma(){
	enableAxiMM2SControlBit(AXI_RESET_BIT);
}

void runAxiMM2SDma(){
	enableAxiMM2SControlBit(AXI_RS_BIT);
}

void stopAxiMM2SDma(){
	disableAxiMM2SControlBit(AXI_RS_BIT);
}

void setAxiDmaAddress(unsigned addr){
	writeValueToAddress(addr, getAxiDmaBaseAddress() + 0x18);
}

void setAxiDmaTransferLength(int length){
	writeValueToAddress(length, getAxiDmaBaseAddress() + 0x28);
}

int getAxiDmaConfigRegister(){
	int value;
	getValueAtAddress(getAxiDmaBaseAddress(), &value);
	return value;
}

int getAxiDmaAddressRegister(){
	int value;
	getValueAtAddress(getAxiDmaBaseAddress() + 0x18, &value);
	return value;
}

int getAxiDmaLengthRegister(){
	int value;
	getValueAtAddress(getAxiDmaBaseAddress() + 0x28, &value);
	return value;
}

int getAxiDmaStatusRegister(){
	int value;
	getValueAtAddress(getAxiDmaBaseAddress() + 0x4, &value);
	return value;
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

int main(void){
	printf("Running memscanner on first 1000 word of system memory");
	unsigned addr = 0;
	int i;
	for(i = 0; i<1000; i++){
		//setup the dma engine
			//enable
		runAxiMM2SDma();
		printf("\nAxi Dma config register after enabling: %08x\n", getAxiDmaConfigRegister());
			//write address
		setAxiDmaAddress(addr);
			//write length as 4 bytes
		setAxiDmaTransferLength(4);
		//poll until transfer complete
		printf("\nAxi Dma config register: %08x\n", getAxiDmaConfigRegister());
		printf("\nAxi Dma Address register: %08x\n", getAxiDmaAddressRegister());
		printf("\nAxi Dma Length register: %08x\n", getAxiDmaLengthRegister());
		printf("\nAxi Dma status register: %08x\n", getAxiDmaStatusRegister());

		while(getAxiDmaStatusRegister() & 2){
			printf("\nAxi Dma status register: %08x\n", getAxiDmaStatusRegister());
		}
		//print address read
		printf("\nRead from address: %08x\n", addr);
		//print output of scanner
		printf("\nCurrent scanner output: %d\n", getMemscannerOutput());
		//print status of scanner
		printf("\nCurrent memory value read: %d\n", getMemscannerMemoryValue());
		//increment address
		addr += 4;
	}
	return 0;
}
