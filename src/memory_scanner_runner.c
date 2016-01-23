#include "xmemory_scanner.h"
#include <time.h>
#include "stdio.h"
#include "math.h"

int main(){
	XMemory_scanner scanner;
	XMemory_scanner_Initialize(&scanner, "memory-scanner");

//	unsigned char search_string[17] = {0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a};
	clock_t begin, end;
	int i;
	XMemory_scanner_Start(&scanner);

/*	XMemory_scanner_Set_search_string_0(&scanner, search_string[0]);
	XMemory_scanner_Set_search_string_0_vld(&scanner);

	XMemory_scanner_Set_search_string_1(&scanner, search_string[1]);
	XMemory_scanner_Set_search_string_1_vld(&scanner);

	XMemory_scanner_Set_search_string_2(&scanner, search_string[2]);
	XMemory_scanner_Set_search_string_2_vld(&scanner);

	XMemory_scanner_Set_search_string_3(&scanner, search_string[3]);
	XMemory_scanner_Set_search_string_3_vld(&scanner);

	XMemory_scanner_Set_search_string_4(&scanner, search_string[4]);
	XMemory_scanner_Set_search_string_4_vld(&scanner);

	XMemory_scanner_Set_search_string_5(&scanner, search_string[5]);
	XMemory_scanner_Set_search_string_5_vld(&scanner);

	XMemory_scanner_Set_search_string_6(&scanner, search_string[6]);
	XMemory_scanner_Set_search_string_6_vld(&scanner);

	XMemory_scanner_Set_search_string_7(&scanner, search_string[7]);
	XMemory_scanner_Set_search_string_7_vld(&scanner);

	XMemory_scanner_Set_search_string_8(&scanner, search_string[8]);
	XMemory_scanner_Set_search_string_8_vld(&scanner);

	XMemory_scanner_Set_search_string_9(&scanner, search_string[9]);
	XMemory_scanner_Set_search_string_9_vld(&scanner);

	XMemory_scanner_Set_search_string_10(&scanner, search_string[10]);
	XMemory_scanner_Set_search_string_10_vld(&scanner);

	XMemory_scanner_Set_search_string_11(&scanner, search_string[11]);
	XMemory_scanner_Set_search_string_11_vld(&scanner);

	XMemory_scanner_Set_search_string_12(&scanner, search_string[12]);
	XMemory_scanner_Set_search_string_12_vld(&scanner);

	XMemory_scanner_Set_search_string_13(&scanner, search_string[13]);
	XMemory_scanner_Set_search_string_13_vld(&scanner);

	XMemory_scanner_Set_search_string_14(&scanner, search_string[14]);
	XMemory_scanner_Set_search_string_14_vld(&scanner);

	XMemory_scanner_Set_search_string_15(&scanner, search_string[15]);
	XMemory_scanner_Set_search_string_15_vld(&scanner);

	XMemory_scanner_Set_search_string_16(&scanner, search_string[16]);
	XMemory_scanner_Set_search_string_16_vld(&scanner);


//	XMemory_scanner_Start(&scanner);
*/
	XMemory_scanner_Search_string_v search_string;
	search_string.word_0 = 0x0a0a0a0a;
	search_string.word_1 = 0x0a0a0a0a;
	search_string.word_2 = 0x0a0a0a0a;
	search_string.word_3 = 0x0a0a0a0a;

	XMemory_scanner_Set_search_string_V(&scanner, search_string);
	XMemory_scanner_Set_search_string_V_vld(&scanner);


	double elapsed_secs;
	int elapsed_secs_int, elapsed_secs_int_last = 0;

	begin = clock();

	printf("Waiting for fabric\n");

	while(XMemory_scanner_Get_count_out_vld(&scanner) != 1){
		elapsed_secs_int = ((clock() - begin)) / CLOCKS_PER_SEC;
		if(elapsed_secs_int_last != elapsed_secs_int && elapsed_secs_int % 10 == 0){
			printf("%i s\n", elapsed_secs_int);
			elapsed_secs_int_last = elapsed_secs_int;
		}
	}

	end = clock();

	u32 fabric_count = XMemory_scanner_Get_count_out(&scanner);

	elapsed_secs = ((double)(end - begin)) / CLOCKS_PER_SEC;

	printf("Elapsed time in hardware: %f s\n", elapsed_secs);

	printf("Fabric count: %i\n", fabric_count);

	printf("Original search string: 0x");
//	for(i = 0; i<17; i++){
//		printf("%02x", search_string);
//	}
	printf("%08x", search_string.word_0);
	printf("%08x", search_string.word_1);
	printf("%08x", search_string.word_2);
	printf("%08x", search_string.word_3);

	XMemory_scanner_Release(&scanner);
}
