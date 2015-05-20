#include "user_mmap_driver.h"
#include "xqam_runner.h"
#include <iostream>
#include <fstream>
#include <math.h>

#define SHARED_MEM_BASE 0x1F400000

using namespace std;


int main(){
	union float16{
		float f;
		u32 u;
	};

	XQam_runner qam_instance;
	if(XQam_runner_Initialize(&qam_instance, "qam") != XST_SUCCESS){
		cout << "Could not initialize qam uio device" << endl;
		return -1;
	}
	ifstream fp_cin ("data/cin.txt");
	ofstream fp_dout ("hw_dout.txt");
	ofstream fp_pout ("hw_pout.txt");
	ofstream fp_debug ("debug.txt");
	ofstream fp_dout_mem("hw_dout_mem.txt");
	float f_din;
	float16 din_i;
	float16 din_q;
	float16 ph_xy_i;
	float16 ph_xy_q;


	float16 dout_mix_i;
	float16 dout_mix_q;
	float16 ph_out_i;
	ph_out_i.u = 0;
	float16 ph_out_q;
	ph_out_q.u = 0;
	float16 loop_integ;
	char wait;

	shared_memory shared_system_mem = getSharedMemoryArea((unsigned)SHARED_MEM_BASE, (unsigned)0x1000);//getUioMemoryArea("/dev/uio1", 0x80000);

	int length = 10;

	for (int i = 0; i<length*4; i+=4) {
		dout_mix_i.u=0;
		dout_mix_q.u = 0;
//		cout << "-------------------------------------"<<endl;
//		cout << "ITERATION: "<<i<<endl;
		fp_cin >> f_din; 
//		cout << "D_i: " << f_din;
		din_i.f = f_din;
		fp_cin >> f_din;

//		cout << " D_q: " << f_din << endl;
		cout << "D_i float: " << din_i.f;
		din_q.f = f_din;
		cout << " D_q float: " << din_q.f << endl;
		cout << "D_i u: " << din_q.u << " D_q u: " << din_i.u << endl;

//		((u32*)shared_system_mem->ptr)[i] = din_i.u;
		writeValueToAddress(din_i.u, SHARED_MEM_BASE + i + 3);
//		cout << "Memory value: " << ((unsigned*)(shared_system_mem->ptr))[i] << endl;
		unsigned memval;
		getValueAtAddress(SHARED_MEM_BASE + i, &memval);
		float memfloat = *((float*)&memval);
		printf("\nMemory value at %08x: %i, as float: %f\n", SHARED_MEM_BASE+i, memval, memfloat);
		((u32*)shared_system_mem->ptr)[i+1] = din_q.u;
		writeValueToAddress(din_q.u, SHARED_MEM_BASE + i + 2);
		writeValueToAddress(din_i.u, SHARED_MEM_BASE + i + 1);
		writeValueToAddress(din_q.u, SHARED_MEM_BASE + i + 0);
	//	u16 fixed_i = createFixed(din_i.f);
	//	u16 fixed_q = createFixed(din_q.f);
//		ph_xy_i.f = ph_out_i.f;
//		ph_xy_q.f = ph_out_q.f;


//		XQam_runner_Start(&qam_instance);
//		cout << "Waiting";
//		while(!XQam_runner_IsDone(&qam_instance)){
//			cout << ".";
//		}
//		cout << endl;

//		dout_mix_i.u = XQam_runner_Get_output_d_i(&qam_instance);
//		dout_mix_q.u = XQam_runner_Get_output_d_q(&qam_instance);

//		ph_out_i.u = XQam_runner_Get_output_ph_i(&qam_instance);
//		ph_out_q.u = XQam_runner_Get_output_ph_q(&qam_instance);

//		loop_integ.u = XQam_runner_Get_loop_out(&qam_instance);

//		fp_dout << dout_mix_i.f << "\t" << dout_mix_q.f << "\t" << " " << endl;
//		fp_pout << ph_out_i.f << "\t" << ph_out_q.f << "\t" << " " << endl;
//		fp_debug << loop_integ.f << "\t" << endl;
//		cout << "D_out_i u32: " << dout_mix_i.u << " D_out_q u32: " << dout_mix_q.u << endl;
//		cout << "D_out_i float: " << dout_mix_i.f << " D_out_q float: " << dout_mix_q.f << endl;
//		cout << "Ph_out_i u32: " << ph_out_i.u << " Ph_out_q u32: " << ph_out_q.u << endl;
//		cout << "Ph_out_i float: " << ph_out_i.f << " Ph_out_q float: " << ph_out_q.f << endl;
//		cout << endl;
	}

	XQam_runner qam_runner;
	if(XQam_runner_Initialize(&qam_runner, "qam") != XST_SUCCESS){
		printf("\nCould not initialize QAM device");
		return -1;
	}
	unsigned sourceAddress = SHARED_MEM_BASE;
	unsigned offset = length*16;
	unsigned destinationAddress = sourceAddress + 0x100;
	printf("\nDest address: 0x%08x, Size of float: %i", destinationAddress, sizeof(float));
	printf("\nSize of unsigned: %i, Size of u32: %i", sizeof(unsigned), sizeof(u32));

	XQam_runner_Start(&qam_runner);

	XQam_runner_Set_sourceAddress(&qam_runner, sourceAddress );
	XQam_runner_Set_sourceAddress_vld(&qam_runner);

	XQam_runner_Set_destinationAddress(&qam_runner, destinationAddress);
	XQam_runner_Set_destinationAddress_vld(&qam_runner);

	XQam_runner_Set_iterations(&qam_runner, length);
	XQam_runner_Set_iterations_vld(&qam_runner);

	XQam_runner_Set_control_in_qam(&qam_runner, 0);
	XQam_runner_Set_control_in_qam_vld(&qam_runner);

	XQam_runner_Set_control_in_lf_p(&qam_runner, 6);
	XQam_runner_Set_control_in_lf_p_vld(&qam_runner);

	XQam_runner_Set_control_in_lf_i(&qam_runner, 5);
	XQam_runner_Set_control_in_lf_i_vld(&qam_runner);

	XQam_runner_Set_control_in_lf_out_gain(&qam_runner, 1);
	XQam_runner_Set_control_in_lf_out_gain_vld(&qam_runner);

	XQam_runner_Set_control_in_reg_clr(&qam_runner, 0);
	XQam_runner_Set_control_in_reg_clr_vld(&qam_runner);

	XQam_runner_Set_control_in_reg_init(&qam_runner, 0);
	XQam_runner_Set_control_in_reg_init_vld(&qam_runner);

	printf("\nWaiting for fabric\n");
	while(XQam_runner_IsDone(&qam_runner) != 1){}

	shared_memory mem2 = getSharedMemoryArea(destinationAddress, 0x1000);

	for(int i=0; i<length*4; i+=4){

		float16 i_out;
		float16 q_out;
		float16 ph_i_out;
		float16 ph_q_out;

		float16 i_out_mem;
		float16 q_out_mem;
		float16 ph_i_out_mem;
		float16 ph_q_out_mem;

		i_out.u = ((u32*)shared_system_mem->ptr)[length*4 + i + 3];
		q_out.u = ((u32*)shared_system_mem->ptr)[length*4 + i +2];
		ph_i_out.u = ((u32*)shared_system_mem->ptr)[length*4 + i + 1];
		ph_q_out.u = ((u32*)shared_system_mem->ptr)[length*4 + i + 0];
		fp_dout << i_out.f << "\t " << q_out.f << "\t "  << ph_i_out.f << "\t" << ph_q_out.f << endl;

		unsigned i_out_mem_u;
		getValueAtAddress(destinationAddress+i+0, &i_out_mem_u);
		unsigned q_out_mem_u;
		getValueAtAddress(destinationAddress+i+1, &q_out_mem_u);
		unsigned i_ph_out_mem_u;
		getValueAtAddress(destinationAddress+i+2, &i_ph_out_mem_u);
		unsigned q_ph_out_mem_u;
		getValueAtAddress(destinationAddress+i+3, &q_ph_out_mem_u);

		i_out_mem.u = i_out_mem_u;// ((u32*)(mem2->ptr))[i];
		q_out_mem.u = q_out_mem_u;//((u32*)(mem2->ptr))[i +1];
		ph_i_out_mem.u = i_ph_out_mem_u;//((u32*)(mem2->ptr))[i + 2];
		ph_q_out_mem.u = q_ph_out_mem_u;//((u32*)(mem2->ptr))[i + 3];
		fp_dout_mem << i_out_mem.f << "\t " << q_out_mem.f << "\t "  << ph_i_out_mem.f << "\t" << ph_q_out_mem.f << endl;
	}

	fp_dout_mem.close();
	fp_dout.close();
	fp_debug.close();
	fp_cin.close();

	cleanupSharedMemoryPointer(shared_system_mem);
	cleanupSharedMemoryPointer(mem2);
	XQam_runner_Release(&qam_runner);
}
