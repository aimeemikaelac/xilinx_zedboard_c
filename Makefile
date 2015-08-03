CFLAGS		:= -static
LIBS 		:= -lcrypto -lssl -lstdc++ -lm -ldl
INCLUDES	:= test_direct_dma_uio_driver user_mmap_driver xilinx_aes_uio_driver xilinx_qam_uio_driver fixed_point xilinx_axi_reset_uio_driver aes_fpga
INCL		:= $(foreach d, $(INCLUDES), -I$d/)
DRIVERS		:= $(foreach d, $(INCLUDES), $(wildcard $d/*.c))
AES 		:= aes_runner.c
TEST_DIRECT 	:= direct_dma_test.c
GPIO 		:= gpio-dev-mem-test.c
QAM		:= qam_runner.cpp
FPGA		:= aes_fpga_test.c

CC		:= gcc -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CPP		:= g++ -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast

aes: $(AES)
	$(CC) -o $@.o $^ $(CFLAGS) $(DRIVERS) $(INCL) $(LIBS)

test_direct: $(TEST_DIRECT)
	$(CC) -o $@.o $^ $(CFLAGS) $(DRIVERS) $(INCL) $(LIBS)

gpio: $(GPIO)
	$(CC) -o $@.o $^ $(CFLAGS) $(DRIVERS) $(INCL) $(LIBS)

qam: $(QAM)
	$(CC) -o $@.o $^ $(CFLAGS) $(DRIVERS) $(INCL) $(LIBS)

fpga: $(FPGA)
	$(CC) -o $@.o $^ $(CFLAGS) $(DRIVERS) $(INCL) $(LIBS)
	
.PHONY: clean aes test_direct gpio qam fpga

clean:
	rm -f *.o
