.SECONDEXPANSION:
#SHELL = /bin/bash

SRC_DIR		:= src
OUT_DIR		:= output
LIB		:= libuio.so
TARGET		:= $(OUT_DIR)/$(LIB)

EXECFLAGS	:= -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -static
CFLAGS		:= -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast #-MMD -include $(OBJFILES:.o=.d)
LDFLAGS		:= -fPIC -c
SHRFLAGS	:= -shared -Wl,-soname,libuio.so
LIBS 		:= -lcrypto -lssl -lstdc++ -lm -ldl -lc
INCLUDES	:= $(SRC_DIR)/user_mmap_driver $(SRC_DIR)/xilinx_aes_uio_driver $(SRC_DIR)/xilinx_qam_uio_driver $(SRC_DIR)/memmgr $(SRC_DIR)/fixed_point $(SRC_DIR)/xilinx_axi_reset_uio_driver $(SRC_DIR)/aes_fpga #$(SRC_DIR)/test_direct_dma_uio_driver 
INCL		:= $(foreach d, $(INCLUDES), -I$d/)
DRIVERS		:= $(foreach d, $(INCLUDES), $(wildcard $d/*.c))
AES 		:= $(SRC_DIR)/aes_runner.c
TEST_DIRECT 	:= $(SRC_DIR)/direct_dma_test.c
GPIO 		:= $(SRC_DIR)/gpio-dev-mem-test.c
QAM		:= $(SRC_DIR)/qam_runner.cpp
FPGA		:= $(SRC_DIR)/aes_fpga_test.c
AES_FPGA	:= $(SRC_DIR)/aes_fpga.c
AES_MEMMGR	:= $(SRC_DIR)/memmgr_aes_test.c

SOURCES		:= $(wildcard $(SRC_DIR)/**/*.c*)# $(SRC_DIR)/*.c*)
SRCS		:= $(addprefix $(OUT_DIR)/,$(notdir $(SOURCES)))
OBJECTS		:= $(addprefix $(OUT_DIR)/,$(notdir $(SOURCES:.c=.o)))
#OBJECTS		:= $(patsubt %/%%/%,$(OUT_DIR)/%,$(_OBJS))


CC		:= gcc 
CPP		:= g++ -fpic -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
AR		:= ar rcs


.PHONY: clean

all: lib

aes: $(AES)
	$(CC) -o $(OUT_DIR)/$@.o $^ $(EXECFLAGS) $(DRIVERS) $(INCL) $(LIBS)

aes_memmgr: $(AES_MEMMGR)
	$(CC) -o $(OUT_DIR)/$@.o $^ $(EXECFLAGS) $(DRIVERS) $(INCL) $(LIBS)

gpio: $(GPIO)
	$(CC) -o $(OUT_DIR)/$@.o $^ $(EXECFLAGS) $(DRIVERS) $(INCL) $(LIBS)

qam: $(QAM)
	$(CC) -o $(OUT_DIR)/$@.o $^ $(EXECFLAGS) $(DRIVERS) $(INCL) $(LIBS)

fpga: $(FPGA)
	$(CC) -o $(OUT_DIR)/$@.o $^ $(EXECFLAGS) $(DRIVERS) $(INCL) $(LIBS)

aes_fpga: $(AES_FPGA)
	$(CC) -o $(OUT_DIR)/$@.o $^ $(EXECFLAGS) $(DRIVERS) $(INCL) $(LIBS)

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

clean_lib:
	rm -rf *.a
	rm -rf *.so
	rm -rf $(OUT_DIR)/*.a
	rm -rf $(OUT_DIR)/*.so

clean: clean_object clean_lib

