#include "xcrec.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(){
	ifstream fp_cin ("data/cin.txt");
	ofstream fp_dout ("hw_dout.txt");
	ofstream fp_debug ("debug.txt");
	float f_din;
	u32 din_i;
	u32 din_q;
	u32 ph_xy_i;
	u32 ph_xy_q;

	u32 dout_mix_i;
	u32 dout_mix_q;
	u32 ph_out_i;
	u32 ph_out_q;
	u32 loop_integ;
	
	XCrec qam_instance;
	if(XCrec_Initialize(&qam_instance, "qam") != XST_SUCCESS){
		cout << "Could not initialize qam uio device" << endl;
		return -1;
	}
	for (int i = 0; i<13000; ++i) {
		fp_cin >> f_din; 
		din_i = f_din;
		fp_cin >> f_din; 
		din_q = f_din;

		ph_xy_i = 0;
		ph_xy_q = 0;

		XCrec_Set_din_i_V(&qam_instance, din_i);
		XCrec_Set_din_q_V(&qam_instance, din_q);
		XCrec_Set_ph_in_i_V(&qam_instance, ph_xy_i);
		XCrec_Set_ph_in_q_V(&qam_instance, ph_xy_q);

		XCrec_Set_control_qam_V(&qam_instance, 0);
		XCrec_Set_control_lf_p(&qam_instance, 6);
		XCrec_Set_control_lf_i(&qam_instance, 5);
		XCrec_Set_control_lf_out_gain(&qam_instance, 1);
		XCrec_Set_control_reg_clr(&qam_instance, 0);
		XCrec_Set_control_reg_init_V(&qam_instance, 0);

		dout_mix_i = XCrec_Get_dout_mix_i_V(&qam_instance);
		dout_mix_q = XCrec_Get_dout_mix_q_V(&qam_instance);
		ph_out_i = XCrec_Get_ph_out_i_V(&qam_instance);
		ph_out_q = XCrec_Get_ph_out_q_V(&qam_instance);
		loop_integ = XCrec_Get_loop_integ_V(&qam_instance);

		fp_dout << dout_mix_i << "\t" << dout_mix_q << "\t" << " " << endl;
		fp_debug << loop_integ << "\t" << endl;
	}

	fp_dout.close();
	fp_debug.close();
	fp_cin.close();

	XCrec_Release(&qam_instance);
}
