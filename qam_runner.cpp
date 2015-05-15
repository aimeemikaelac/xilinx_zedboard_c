#include "xqam_runner.h"
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

u16 createFixed(float in){
	int i;
	int sign;
	if(in < 0){
		sign = -1;
	} else{
		sign = 1;
	}
	u16 val= 0;
	if(sign == -1){
		val = 1;
		val = val <<15;
	}
	int intval = (int)in;
	intval &= 0x3;
	float fract = sign*in - intval;
	cout << "Sign: "<<sign <<endl;
	cout << "Fract: "<<fract<<endl;
	intval = intval << 13;
	int factor = 1;
	factor = factor << 13;
	cout << "Factor: " << factor << endl;
	fract *= (float)factor;
	cout << "Fract: "<<fract<<endl;
	int fractval = (int)ceil(fract);
	cout << "Input float: " << in <<endl;
	val = intval + fractval;
	val *= sign;
	cout << "Output fixed point: " << val << endl;
	printf("Output fixed point: 0x%04x\n", val);
	cout << "Fract part: " << fractval << endl;
	printf("Fract part: 0x%04x\n", fractval);
	cout << "Int part: " << int(in) << endl;
	return val;
}

float createFloat(u16 in){
	int sign;
	cout << "Input fixed point: "<<in<<endl;
	if(in & 0x8000){
		sign = -1;
	} else{
		sign = 1;
	}
	int factor = 1;
	factor = factor << 13;
	int intval = (in & 0x6000);
	int fractval = 0;
	fractval = (in & 0x1fff);
	cout << "Fractval: "<<fractval<<endl;
	float fract = (float)fractval;
	fract = fract / (float)factor;
	cout << "Fract val: "<<fract<<endl;
	intval = intval >> 13;
	float output = (float)intval + fract;
	cout << "Input fixed point: "<<in<<endl;
	printf("Input fixed point: 0x%08x\n", in);
	cout << "Output float: "<<output<<endl;
	return output;
}

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

	for (int i = 0; i<20; i++){//13000; ++i) {
		dout_mix_i.u=0;
		dout_mix_q.u = 0;
		cout << "-------------------------------------"<<endl;
		cout << "ITERATION: "<<i<<endl;
		fp_cin >> f_din; 
		cout << "D_i: " << f_din;
		din_i.f = f_din;
		fp_cin >> f_din;
		cout << " D_q: " << f_din << endl;
		cout << "D_i float: " << din_i.f;
		din_q.f = f_din;
		cout << " D_q float: " << din_q.f << endl;
		cout << "D_i u: " << din_q.u << " D_q u: " << din_i.u << endl;

	//	u16 fixed_i = createFixed(din_i.f);
	//	u16 fixed_q = createFixed(din_q.f);
		ph_xy_i.f = ph_out_i.f;
		ph_xy_q.f = ph_out_q.f;

//		XCrec_Set_din_i_V(&qam_instance, fixed_i);
//		XCrec_Set_din_q_V(&qam_instance, fixed_q);
//		XCrec_Set_ph_in_i_V(&qam_instance, ph_xy_i.u);
//		XCrec_Set_ph_in_q_V(&qam_instance, ph_xy_q.u);

//		XCrec_Set_control_qam_V(&qam_instance, 0);
//		XCrec_Set_control_lf_p(&qam_instance, 6);
//		XCrec_Set_control_lf_i(&qam_instance, 5);
//		XCrec_Set_control_lf_out_gain(&qam_instance, 1);
//		XCrec_Set_control_reg_clr(&qam_instance, 0);
//		XCrec_Set_control_reg_init_V(&qam_instance, 0);
		XQam_runner_Start(&qam_instance);

		XQam_runner_Set_input_d_i(&qam_instance, din_i.u);

		XQam_runner_Set_input_d_q(&qam_instance, din_q.u);

		XQam_runner_Set_input_ph_i(&qam_instance, ph_xy_i.u);

		XQam_runner_Set_input_ph_q(&qam_instance, ph_xy_q.u);


		XQam_runner_Set_control_in_qam(&qam_instance, 0);

		XQam_runner_Set_control_in_lf_p(&qam_instance, 6);

		XQam_runner_Set_control_in_lf_i(&qam_instance, 5);

		XQam_runner_Set_control_in_lf_out_gain(&qam_instance, 1);

		XQam_runner_Set_control_in_reg_clr(&qam_instance, 0);

		XQam_runner_Set_control_in_reg_init(&qam_instance, 0);


		XQam_runner_Set_input_d_i_vld(&qam_instance);
		XQam_runner_Set_input_d_q_vld(&qam_instance);
		XQam_runner_Set_input_ph_i_vld(&qam_instance);
		XQam_runner_Set_input_ph_q_vld(&qam_instance);

		XQam_runner_Set_control_in_reg_init_vld(&qam_instance);
		XQam_runner_Set_control_in_qam_vld(&qam_instance);
		XQam_runner_Set_control_in_lf_p_vld(&qam_instance);
		XQam_runner_Set_control_in_lf_i_vld(&qam_instance);
		XQam_runner_Set_control_in_lf_out_gain_vld(&qam_instance);
		XQam_runner_Set_control_in_reg_clr_vld(&qam_instance);

//		XQam_runner_Start(&qam_instance);
		cout << "Waiting";
		while(!XQam_runner_IsDone(&qam_instance)){
//			cout << ".";
		}
		cout << endl;

		dout_mix_i.u = XQam_runner_Get_output_d_i(&qam_instance);
		dout_mix_q.u = XQam_runner_Get_output_d_q(&qam_instance);

		ph_out_i.u = XQam_runner_Get_output_ph_i(&qam_instance);
		ph_out_q.u = XQam_runner_Get_output_ph_q(&qam_instance);

		loop_integ.u = XQam_runner_Get_loop_out(&qam_instance);

		fp_dout << dout_mix_i.f << "\t" << dout_mix_q.f << "\t" << " " << endl;
		fp_pout << ph_out_i.f << "\t" << ph_out_q.f << "\t" << " " << endl;
		fp_debug << loop_integ.f << "\t" << endl;
		cout << "D_out_i u32: " << dout_mix_i.u << " D_out_q u32: " << dout_mix_q.u << endl;
		cout << "D_out_i float: " << dout_mix_i.f << " D_out_q float: " << dout_mix_q.f << endl;
		cout << "Ph_out_i u32: " << ph_out_i.u << " Ph_out_q u32: " << ph_out_q.u << endl;
		cout << "Ph_out_i float: " << ph_out_i.f << " Ph_out_q float: " << ph_out_q.f << endl;
		cout << endl;
	}

	fp_dout.close();
	fp_debug.close();
	fp_cin.close();

	XQam_runner_Release(&qam_instance);
}
