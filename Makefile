LIBS 		:= -lcrypto -lssl -lstdc++
INCLUDES	:= test_direct_dma_uio_driver user_mmap_driver xilinx_aes_uio_driver xilinx_qam_uio_driver
INCL		:= $(foreach d, $(INCLUDES), -I$d/)
DRIVERS		:= $(foreach d, $(INCLUDES), $(wildcard $d/*.c))
AES 		:= aes_runner.c
TEST_DIRECT 	:= direct_dma_test.c
GPIO 		:= gpio-dev-mem-test.c
QAM		:= qam_runner.cpp

CC		:= gcc
CPP		:= g++

aes: $(AES)
	$(CC) -o $@.o $^ $(DRIVERS) $(INCL) $(LIBS)

test_direct: $(TEST_DIRECT)
	$(CC) -o $@.o $^ $(DRIVERS) $(INCL) $(LIBS)

gpio: $(GPIO)
	$(CC) -o $@.o $^ $(DRIVERS) $(INCL) $(LIBS)

qam: $(QAM)
	$(CC) -o $@.o $^ $(DRIVERS) $(INCL) $(LIBS)

.PHONY: clean aes test_direct gpio qam

clean:
	rm -f *.o
