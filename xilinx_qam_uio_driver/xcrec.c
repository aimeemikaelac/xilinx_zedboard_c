// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2014.1
// Copyright (C) 2014 Xilinx Inc. All rights reserved.
// 
// ==============================================================

/***************************** Include Files *********************************/
#include "xcrec.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XCrec_CfgInitialize(XCrec *InstancePtr, XCrec_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Axilites_BaseAddress = ConfigPtr->Axilites_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XCrec_Set_din_i_V(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_DIN_I_V_DATA, Data);
}

u32 XCrec_Get_din_i_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_DIN_I_V_DATA);
    return Data;
}

void XCrec_Set_din_q_V(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_DIN_Q_V_DATA, Data);
}

u32 XCrec_Get_din_q_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_DIN_Q_V_DATA);
    return Data;
}

u32 XCrec_Get_dout_mix_i_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_DOUT_MIX_I_V_DATA);
    return Data;
}

u32 XCrec_Get_dout_mix_q_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_DOUT_MIX_Q_V_DATA);
    return Data;
}

void XCrec_Set_ph_in_i_V(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_PH_IN_I_V_DATA, Data);
}

u32 XCrec_Get_ph_in_i_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_PH_IN_I_V_DATA);
    return Data;
}

void XCrec_Set_ph_in_q_V(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_PH_IN_Q_V_DATA, Data);
}

u32 XCrec_Get_ph_in_q_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_PH_IN_Q_V_DATA);
    return Data;
}

u32 XCrec_Get_ph_out_i_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_PH_OUT_I_V_DATA);
    return Data;
}

u32 XCrec_Get_ph_out_q_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_PH_OUT_Q_V_DATA);
    return Data;
}

u32 XCrec_Get_loop_integ_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_LOOP_INTEG_V_DATA);
    return Data;
}

void XCrec_Set_control_qam_V(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_QAM_V_DATA, Data);
}

u32 XCrec_Get_control_qam_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_QAM_V_DATA);
    return Data;
}

void XCrec_Set_control_lf_p(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_LF_P_DATA, Data);
}

u32 XCrec_Get_control_lf_p(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_LF_P_DATA);
    return Data;
}

void XCrec_Set_control_lf_i(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_LF_I_DATA, Data);
}

u32 XCrec_Get_control_lf_i(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_LF_I_DATA);
    return Data;
}

void XCrec_Set_control_lf_out_gain(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_LF_OUT_GAIN_DATA, Data);
}

u32 XCrec_Get_control_lf_out_gain(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_LF_OUT_GAIN_DATA);
    return Data;
}

void XCrec_Set_control_reg_clr(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_REG_CLR_DATA, Data);
}

u32 XCrec_Get_control_reg_clr(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_REG_CLR_DATA);
    return Data;
}

void XCrec_Set_control_reg_init_V(XCrec *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XCrec_WriteReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_REG_INIT_V_DATA, Data);
}

u32 XCrec_Get_control_reg_init_V(XCrec *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XCrec_ReadReg(InstancePtr->Axilites_BaseAddress, XCREC_AXILITES_ADDR_CONTROL_REG_INIT_V_DATA);
    return Data;
}

