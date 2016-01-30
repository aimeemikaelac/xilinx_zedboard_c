// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2014.1
// Copyright (C) 2014 Xilinx Inc. All rights reserved.
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
//        bit 0  - ap_return[0] (Read)
//        others - reserved
// 0x18 : Data signal of search_string_V
//        bit 31~0 - search_string_V[31:0] (Read/Write)
// 0x1c : Data signal of search_string_V
//        bit 31~0 - search_string_V[63:32] (Read/Write)
// 0x20 : Data signal of search_string_V
//        bit 31~0 - search_string_V[95:64] (Read/Write)
// 0x24 : Data signal of search_string_V
//        bit 31~0 - search_string_V[127:96] (Read/Write)
// 0x28 : Control signal of search_string_V
//        bit 0  - search_string_V_ap_vld (Read/Write/SC)
//        others - reserved
// 0x2c : Data signal of count_out
//        bit 31~0 - count_out[31:0] (Read)
// 0x30 : Control signal of count_out
//        bit 0  - count_out_ap_vld (Read/COR)
//        others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XMEMORY_SCANNER_AXILITES_ADDR_AP_CTRL              0x00
#define XMEMORY_SCANNER_AXILITES_ADDR_GIE                  0x04
#define XMEMORY_SCANNER_AXILITES_ADDR_IER                  0x08
#define XMEMORY_SCANNER_AXILITES_ADDR_ISR                  0x0c
#define XMEMORY_SCANNER_AXILITES_ADDR_AP_RETURN            0x10
#define XMEMORY_SCANNER_AXILITES_BITS_AP_RETURN            1
#define XMEMORY_SCANNER_AXILITES_ADDR_SEARCH_STRING_V_DATA 0x18
#define XMEMORY_SCANNER_AXILITES_BITS_SEARCH_STRING_V_DATA 128
#define XMEMORY_SCANNER_AXILITES_ADDR_SEARCH_STRING_V_CTRL 0x28
#define XMEMORY_SCANNER_AXILITES_ADDR_COUNT_OUT_DATA       0x2c
#define XMEMORY_SCANNER_AXILITES_BITS_COUNT_OUT_DATA       32
#define XMEMORY_SCANNER_AXILITES_ADDR_COUNT_OUT_CTRL       0x30
