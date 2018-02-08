.SECONDEXPANSION:
#SHELL = /bin/bash

SRC_DIR		:= src
OUT_DIR		:= output
LIB		:= libuio.so
TARGET		:= $(OUT_DIR)/$(LIB)

EXECFLAGS	:= -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -static
LIBS 		:= -lssl -lcrypto -lm -ldl -lpthread
LDFLAGS		:= $(LIBS)
SHRFLAGS	:= -shared -Wl,-soname,libuio.so
CPP_LIBS	:= -lstdc++ -lc
INCLUDES	:= $(SRC_DIR)/user_mmap_driver $(SRC_DIR)/xilinx_aes_uio_driver $(SRC_DIR)/xilinx_qam_uio_driver $(SRC_DIR)/memmgr $(SRC_DIR)/xilinx_axi_reset_uio_driver $(SRC_DIR)/aes_fpga $(SRC_DIR)/xilinx_memory_scanner_uio_driver $(SRC_DIR)/xilinx_memory_scanner_ddr_uio_driver  $(SRC_DIR)/xilinx_triple_aes_uio_driver $(SRC_DIR)/curve_25519_uio_driver $(SRC_DIR)/crypto_sign_uio_driver $(SRC_DIR)/ed25519
#$(SRC_DIR)/test_direct_dma_uio_driver $(SRC_DIR)/ed25519_ref_c
INCL		:= $(foreach d, $(INCLUDES), -I$d/)
CFLAGS		:= $(INCL) #-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -fPIC -c
CXXFLAGS	:= $(INCL)
DRIVERS		:= $(foreach d, $(INCLUDES), $(wildcard $d/*.c))
AES 		:= $(SRC_DIR)/aes_runner.o
TEST_DIRECT 	:= $(SRC_DIR)/direct_dma_test.o
GPIO 		:= $(SRC_DIR)/gpio-dev-mem-test.o
QAM		:= $(SRC_DIR)/qam_runner.o
FPGA		:= $(SRC_DIR)/aes_fpga_test.o
AES_FPGA	:= $(SRC_DIR)/aes_fpga.o
AES_MEMMGR	:= $(SRC_DIR)/memmgr_aes_test.o
MEM_SCAN	:= $(SRC_DIR)/memory_scanner_runner.o
MEM_DDR		:= $(SRC_DIR)/memory_scanner_runner_ddr.o
TRIPLE_AES	:= $(SRC_DIR)/triple_aes_test.o
BBRAM		:= $(SRC_DIR)/zcu102_program_bbram.o
FILE_MEM	:= $(SRC_DIR)/write_file_to_memory.o
CURVE		:= $(SRC_DIR)/curve_25519_test.o
CRYPTO_SIGN	:= $(SRC_DIR)/crypto_sign_test.o
ED25519		:= $(SRC_DIR)/ed25519_sign_microblaze_test.o
ED25519_TEST:= $(SRC_DIR)/ecdsa_ed25519_microblaze_test.o
WRITE_FILE	:= $(SRC_DIR)/write_file_to_memory.o


SOURCES		:= $(wildcard $(SRC_DIR)/**/*.c*)# $(SRC_DIR)/*.c*)
SRCS		:= $(addprefix $(OUT_DIR)/,$(notdir $(SOURCES)))
OBJECTS		:= $(DRIVERS:.c=.o)
#OBJECTS		:= $(patsubt %/%%/%,$(OUT_DIR)/%,$(_OBJS))


CC		:= gcc #-pg
CPP		:= g++ -fpic -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
AR		:= ar rcs


.PHONY: clean

all: lib

aes: $(AES) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

aes_memmgr: $(AES_MEMMGR) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

memory_scanner: $(MEM_SCAN) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

memory_ddr: $(MEM_DDR) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

gpio: $(GPIO)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

qam: $(QAM) $(OBJECTS)
	$(CXX) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

fpga: $(FPGA) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

aes_fpga: $(AES_FPGA) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

triple_aes: $(TRIPLE_AES) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

bbram: $(BBRAM) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

file_mem: $(FILE_MEM) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

curve: $(CURVE) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

crypto_sign: $(CRYPTO_SIGN) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

# ed25519: $(ED25519)
# 	$(CC) -o $(OUT_DIR)/$@.o $^ $(EXECFLAGS) $(DRIVERS) $(INCL) $(LIBS)

ed25519_test: $(ED25519_TEST) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)

write_file: $(WRITE_FILE) $(OBJECTS)
	$(CC) -o $(OUT_DIR)/$@ $^ $(LDFLAGS)
#$(OUT_DIR)/%.o: $(SOURCES)
#	$(CC) -c -fPIC -o $@ $^ $(LIBS) $(CFLAGS)

COMPILE = $(CC) $(LDFLAGS) $(CFLAGS) -o $(addprefix $(OUT_DIR)/,$(addsuffix .o,$(basename $(notdir $(SRC))))) $(SRC) $(INCL); #$(LIBS);
$(TARGET): out_dir
#	$(info $$SOURCES is [${SOURCES}])
#	$(info $$_OBJS is [${_OBJS}])
#	$(info $$OBJECTS is [${OBJECTS}])
#	$(CPP) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	$(foreach SRC,$(SOURCES),$(COMPILE))
#	$(CC) -fPIC $(LDFLAGS) -Wl,-soname,$(LIB) -o $(TARGET) $(OBJECTS) $(LIBS)
	$(AR) $(addsuffix .a,$(basename $@)) $(OBJECTS)
	$(CC) $(SHRFLAGS) -o $(TARGET) $(OBJECTS)

lib: $(TARGET)

out_dir:
	mkdir -p $(OUT_DIR)

clean_object:
	rm -rf *.o
	rm -rf $(OUT_DIR)/*.o
	rm -rf *.d
	rm -rf $(OUT_DIR)/*.d
	rm -rf $(OBJECTS)

clean_lib:
	rm -rf *.a
	rm -rf *.so
	rm -rf $(OUT_DIR)/*.a
	rm -rf $(OUT_DIR)/*.so
	rm -rf src/*.o

clean: clean_object clean_lib

spotless: clean
	rm -rf output/*
