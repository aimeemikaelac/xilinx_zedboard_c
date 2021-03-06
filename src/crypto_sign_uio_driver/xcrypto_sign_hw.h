// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2017.1
// Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
// 
// ==============================================================

// AXILiteS
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x10 : Data signal of ap_return
//        bit 31~0 - ap_return[31:0] (Read)
// 0x18 : Data signal of num_blocks
//        bit 31~0 - num_blocks[31:0] (Read/Write)
// 0x1c : reserved
// 0x40 ~
// 0x7f : Memory 'sk' (64 * 8b)
//        Word n : bit [ 7: 0] - sk[4n]
//                 bit [15: 8] - sk[4n+1]
//                 bit [23:16] - sk[4n+2]
//                 bit [31:24] - sk[4n+3]
// 0x80 ~
// 0xbf : Memory 'signature_out' (64 * 8b)
//        Word n : bit [ 7: 0] - signature_out[4n]
//                 bit [15: 8] - signature_out[4n+1]
//                 bit [23:16] - signature_out[4n+2]
//                 bit [31:24] - signature_out[4n+3]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XCRYPTO_SIGN_AXILITES_ADDR_AP_CTRL            0x00
#define XCRYPTO_SIGN_AXILITES_ADDR_GIE                0x04
#define XCRYPTO_SIGN_AXILITES_ADDR_IER                0x08
#define XCRYPTO_SIGN_AXILITES_ADDR_ISR                0x0c
#define XCRYPTO_SIGN_AXILITES_ADDR_AP_RETURN          0x10
#define XCRYPTO_SIGN_AXILITES_BITS_AP_RETURN          32
#define XCRYPTO_SIGN_AXILITES_ADDR_NUM_BLOCKS_DATA    0x18
#define XCRYPTO_SIGN_AXILITES_BITS_NUM_BLOCKS_DATA    32
#define XCRYPTO_SIGN_AXILITES_ADDR_SK_BASE            0x40
#define XCRYPTO_SIGN_AXILITES_ADDR_SK_HIGH            0x7f
#define XCRYPTO_SIGN_AXILITES_WIDTH_SK                8
#define XCRYPTO_SIGN_AXILITES_DEPTH_SK                64
#define XCRYPTO_SIGN_AXILITES_ADDR_SIGNATURE_OUT_BASE 0x80
#define XCRYPTO_SIGN_AXILITES_ADDR_SIGNATURE_OUT_HIGH 0xbf
#define XCRYPTO_SIGN_AXILITES_WIDTH_SIGNATURE_OUT     8
#define XCRYPTO_SIGN_AXILITES_DEPTH_SIGNATURE_OUT     64

