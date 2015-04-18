// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2014.1
// Copyright (C) 2014 Xilinx Inc. All rights reserved.
// 
// ==============================================================

#ifndef XTEST_DIRECT_DMA_H
#define XTEST_DIRECT_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xtest_direct_dma_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Axilites_BaseAddress;
} XTest_direct_dma_Config;
#endif

typedef struct {
    u32 Axilites_BaseAddress;
    u32 IsReady;
} XTest_direct_dma;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XTest_direct_dma_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XTest_direct_dma_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XTest_direct_dma_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XTest_direct_dma_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XTest_direct_dma_Initialize(XTest_direct_dma *InstancePtr, u16 DeviceId);
XTest_direct_dma_Config* XTest_direct_dma_LookupConfig(u16 DeviceId);
int XTest_direct_dma_CfgInitialize(XTest_direct_dma *InstancePtr, XTest_direct_dma_Config *ConfigPtr);
#else
int XTest_direct_dma_Initialize(XTest_direct_dma *InstancePtr, const char* InstanceName);
int XTest_direct_dma_Release(XTest_direct_dma *InstancePtr);
#endif


void XTest_direct_dma_Set_sourceAddress(XTest_direct_dma *InstancePtr, u32 Data);
u32 XTest_direct_dma_Get_sourceAddress(XTest_direct_dma *InstancePtr);
void XTest_direct_dma_Set_sourceAddress_vld(XTest_direct_dma *InstancePtr);
u32 XTest_direct_dma_Get_sourceAddress_vld(XTest_direct_dma *InstancePtr);
void XTest_direct_dma_Set_destinationAddress(XTest_direct_dma *InstancePtr, u32 Data);
u32 XTest_direct_dma_Get_destinationAddress(XTest_direct_dma *InstancePtr);
void XTest_direct_dma_Set_destinationAddress_vld(XTest_direct_dma *InstancePtr);
u32 XTest_direct_dma_Get_destinationAddress_vld(XTest_direct_dma *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
