#include <string.h>
#include <stdint.h>
#include "user_mmap_driver.h"
#include "xcurve25519_donna_hw.h"

#ifdef _MSC_VER
#define inline __inline
#endif

typedef uint8_t u8;
typedef int32_t s32;
typedef int64_t limb;

#define CURVE_BASE_ADDR 0xA0000000

int
curve25519_donna(u8 mypublic[32], const u8 secret[32], const u8 basepoint[32]){
	int i;
	unsigned int ap_done;
	unsigned int *control_register;
	shared_memory curve_device = getSharedMemoryArea(CURVE_BASE_ADDR, 0x1000);
	unsigned char *data_ptr = (unsigned char*)curve_device->ptr;
	control_register = (unsigned int*)data_ptr;

	unsigned int *data_public =
		(unsigned int*)(data_ptr + XCURVE25519_DONNA_AXILITES_ADDR_MYPUBLIC_IN_BASE);
	unsigned int *data_secret =
		(unsigned int*)(data_ptr + XCURVE25519_DONNA_AXILITES_ADDR_SECRET_IN_BASE);
	unsigned int *data_basepoint =
		(unsigned int*)(data_ptr + XCURVE25519_DONNA_AXILITES_ADDR_BASEPOINT_IN_BASE);

	unsigned int *data_public_out =
		(unsigned int*)(data_ptr + XCURVE25519_DONNA_AXILITES_ADDR_MYPUBLIC_OUT_BASE);
	unsigned int *data_secret_out =
		(unsigned int*)(data_ptr + XCURVE25519_DONNA_AXILITES_ADDR_SECRET_OUT_BASE);
	unsigned int *data_basepoint_out =
		(unsigned int*)(data_ptr + XCURVE25519_DONNA_AXILITES_ADDR_BASEPOINT_OUT_BASE);

	for(i=0; i<32/4; i++){
		data_public[i] = ((unsigned int*)(mypublic))[i];
		data_secret[i] = ((unsigned int*)(secret))[i];
		data_basepoint[i] = ((unsigned int*)(basepoint))[i];
	}

	// data_ptr[0] = 1;
	*control_register = 1;

	while((ap_done = (*control_register) &0x2) == 0){
		__asm__("");
		asm("");
		//spin wait
		// control_register = device_memory[0];
	}
	printf("XCurve done\n");

	for(i=0; i<32/4; i++){
		((unsigned int*)(mypublic))[i] = data_public_out[i];
		((unsigned int*)(secret))[i] = data_secret_out[i];
		((unsigned int*)(basepoint))[i]= data_basepoint_out[i];
	}

	return 0;
}

int
curve25519_donna_ref(u8 *mypublic, const u8 *secret, const u8 *basepoint);

unsigned char e1k[32];
unsigned char e2k[32];
unsigned char e1e2k[32];
unsigned char e2e1k[32];
unsigned char e1[32] = {3};
unsigned char e2[32] = {5};
unsigned char k[32] = {9};

unsigned char e1_2[32] = {3};
unsigned char e2_2[32] = {5};
unsigned char k_2[32] = {9};

void test_reference(unsigned char *ek,unsigned char *e,unsigned char *k){
	int i;

	for (i = 0;i < 32;++i){
		printf("%02x",(unsigned int) e[i]);
	}
	printf("\n");
	for (i = 0;i < 32;++i){
		printf("%02x",(unsigned int) k[i]);
	}
	printf("\n");

	curve25519_donna_ref(ek,e,k);

	for (i = 0;i < 32;++i){
		printf("%02x",(unsigned int) ek[i]);
	}
	printf("\n");
}

void test_hls(unsigned char *ek,unsigned char *e,unsigned char *k){
	int i;


	curve25519_donna(ek,e,k);

	for (i = 0;i < 32;++i){
		printf("%02x",(unsigned int) e[i]);
	}
	printf("\n");
	for (i = 0;i < 32;++i){
		printf("%02x",(unsigned int) k[i]);
	}
	printf("\n");
	for (i = 0;i < 32;++i){
		printf("%02x",(unsigned int) ek[i]);
	}
	printf("\n");
}

int main(){
	int i;
	unsigned char e1e2k_ref[32];
	unsigned char e2e1k_ref[32];
	printf("Testing ref:\n");

    memset(e1k, 0, 32);
    memset(e2e1k, 0, 32);
    memset(e2k, 0, 32);
    memset(e1e2k, 0, 32);

	test_reference(e1k,e1,k);
	test_reference(e2e1k,e2,e1k);
	test_reference(e2k,e2,k);
	test_reference(e1e2k,e1,e2k);

	for(i=0; i<32; i++){
		if(e1e2k[i] != e2e1k[i]){
			printf("Reference failed\n");
			return -2;
		}
	}

	memcpy(e1e2k_ref, e1e2k, 32);
	memcpy(e2e1k_ref, e2e1k, 32);

    memset(e1k, 0, 32);
    memset(e2e1k, 0, 32);
    memset(e2k, 0, 32);
    memset(e1e2k, 0, 32);

	printf("\nTesting hls:\n");
	test_hls(e1k,e1_2,k_2);
	test_hls(e2e1k,e2_2,e1k);
	test_hls(e2k,e2_2,k_2);
	test_hls(e1e2k,e1_2,e2k);

	for(i=0; i<32; i++){
		if(e1e2k[i] != e2e1k[i]){
			printf("HLS failed self-test\n");
			return -2;
		}
	}

	for(i=0; i<32; i++){
		if(e1e2k[i] != e1e2k_ref[i]){
			printf("e1e2k comparison failed\n");
			return -1;
		}
		if(e2e1k[i] != e2e1k_ref[i]){
			printf("e2e1k comparison failed\n");
			return -1;
		}
	}

	return 0;
}




/* Field element representation:
 *
 * Field elements are written as an array of signed, 64-bit limbs, least
 * significant first. The value of the field element is:
 *   x[0] + 2^26·x[1] + x^51·x[2] + 2^102·x[3] + ...
 *
 * i.e. the limbs are 26, 25, 26, 25, ... bits wide. */

/* Sum two numbers: output += in */
static void fsum(limb *output, const limb *in) {
  unsigned i;
  for (i = 0; i < 10; i += 2) {
    output[0+i] = output[0+i] + in[0+i];
    output[1+i] = output[1+i] + in[1+i];
  }
}

/* Find the difference of two numbers: output = in - output
 * (note the order of the arguments!). */
static void fdifference(limb *output, const limb *in) {
  unsigned i;
  for (i = 0; i < 10; ++i) {
    output[i] = in[i] - output[i];
  }
}

/* Multiply a number by a scalar: output = in * scalar */
static void fscalar_product(limb *output, const limb *in, const limb scalar) {
  unsigned i;
  for (i = 0; i < 10; ++i) {
    output[i] = in[i] * scalar;
  }
}

/* Multiply two numbers: output = in2 * in
 *
 * output must be distinct to both inputs. The inputs are reduced coefficient
 * form, the output is not.
 *
 * output[x] <= 14 * the largest product of the input limbs. */
static void fproduct(limb *output, const limb *in2, const limb *in) {
  output[0] =       ((limb) ((s32) in2[0])) * ((s32) in[0]);
  output[1] =       ((limb) ((s32) in2[0])) * ((s32) in[1]) +
                    ((limb) ((s32) in2[1])) * ((s32) in[0]);
  output[2] =  2 *  ((limb) ((s32) in2[1])) * ((s32) in[1]) +
                    ((limb) ((s32) in2[0])) * ((s32) in[2]) +
                    ((limb) ((s32) in2[2])) * ((s32) in[0]);
  output[3] =       ((limb) ((s32) in2[1])) * ((s32) in[2]) +
                    ((limb) ((s32) in2[2])) * ((s32) in[1]) +
                    ((limb) ((s32) in2[0])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[3])) * ((s32) in[0]);
  output[4] =       ((limb) ((s32) in2[2])) * ((s32) in[2]) +
               2 * (((limb) ((s32) in2[1])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[3])) * ((s32) in[1])) +
                    ((limb) ((s32) in2[0])) * ((s32) in[4]) +
                    ((limb) ((s32) in2[4])) * ((s32) in[0]);
  output[5] =       ((limb) ((s32) in2[2])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[3])) * ((s32) in[2]) +
                    ((limb) ((s32) in2[1])) * ((s32) in[4]) +
                    ((limb) ((s32) in2[4])) * ((s32) in[1]) +
                    ((limb) ((s32) in2[0])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[5])) * ((s32) in[0]);
  output[6] =  2 * (((limb) ((s32) in2[3])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[1])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[5])) * ((s32) in[1])) +
                    ((limb) ((s32) in2[2])) * ((s32) in[4]) +
                    ((limb) ((s32) in2[4])) * ((s32) in[2]) +
                    ((limb) ((s32) in2[0])) * ((s32) in[6]) +
                    ((limb) ((s32) in2[6])) * ((s32) in[0]);
  output[7] =       ((limb) ((s32) in2[3])) * ((s32) in[4]) +
                    ((limb) ((s32) in2[4])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[2])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[5])) * ((s32) in[2]) +
                    ((limb) ((s32) in2[1])) * ((s32) in[6]) +
                    ((limb) ((s32) in2[6])) * ((s32) in[1]) +
                    ((limb) ((s32) in2[0])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[7])) * ((s32) in[0]);
  output[8] =       ((limb) ((s32) in2[4])) * ((s32) in[4]) +
               2 * (((limb) ((s32) in2[3])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[5])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[1])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[7])) * ((s32) in[1])) +
                    ((limb) ((s32) in2[2])) * ((s32) in[6]) +
                    ((limb) ((s32) in2[6])) * ((s32) in[2]) +
                    ((limb) ((s32) in2[0])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[0]);
  output[9] =       ((limb) ((s32) in2[4])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[5])) * ((s32) in[4]) +
                    ((limb) ((s32) in2[3])) * ((s32) in[6]) +
                    ((limb) ((s32) in2[6])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[2])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[7])) * ((s32) in[2]) +
                    ((limb) ((s32) in2[1])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[1]) +
                    ((limb) ((s32) in2[0])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[0]);
  output[10] = 2 * (((limb) ((s32) in2[5])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[3])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[7])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[1])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[1])) +
                    ((limb) ((s32) in2[4])) * ((s32) in[6]) +
                    ((limb) ((s32) in2[6])) * ((s32) in[4]) +
                    ((limb) ((s32) in2[2])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[2]);
  output[11] =      ((limb) ((s32) in2[5])) * ((s32) in[6]) +
                    ((limb) ((s32) in2[6])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[4])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[7])) * ((s32) in[4]) +
                    ((limb) ((s32) in2[3])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[3]) +
                    ((limb) ((s32) in2[2])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[2]);
  output[12] =      ((limb) ((s32) in2[6])) * ((s32) in[6]) +
               2 * (((limb) ((s32) in2[5])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[7])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[3])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[3])) +
                    ((limb) ((s32) in2[4])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[4]);
  output[13] =      ((limb) ((s32) in2[6])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[7])) * ((s32) in[6]) +
                    ((limb) ((s32) in2[5])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[5]) +
                    ((limb) ((s32) in2[4])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[4]);
  output[14] = 2 * (((limb) ((s32) in2[7])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[5])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[5])) +
                    ((limb) ((s32) in2[6])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[6]);
  output[15] =      ((limb) ((s32) in2[7])) * ((s32) in[8]) +
                    ((limb) ((s32) in2[8])) * ((s32) in[7]) +
                    ((limb) ((s32) in2[6])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[6]);
  output[16] =      ((limb) ((s32) in2[8])) * ((s32) in[8]) +
               2 * (((limb) ((s32) in2[7])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[7]));
  output[17] =      ((limb) ((s32) in2[8])) * ((s32) in[9]) +
                    ((limb) ((s32) in2[9])) * ((s32) in[8]);
  output[18] = 2 *  ((limb) ((s32) in2[9])) * ((s32) in[9]);
}

/* Reduce a long form to a short form by taking the input mod 2^255 - 19.
 *
 * On entry: |output[i]| < 14*2^54
 * On exit: |output[0..8]| < 280*2^54 */
static void freduce_degree(limb *output) {
  /* Each of these shifts and adds ends up multiplying the value by 19.
   *
   * For output[0..8], the absolute entry value is < 14*2^54 and we add, at
   * most, 19*14*2^54 thus, on exit, |output[0..8]| < 280*2^54. */
  output[8] += output[18] << 4;
  output[8] += output[18] << 1;
  output[8] += output[18];
  output[7] += output[17] << 4;
  output[7] += output[17] << 1;
  output[7] += output[17];
  output[6] += output[16] << 4;
  output[6] += output[16] << 1;
  output[6] += output[16];
  output[5] += output[15] << 4;
  output[5] += output[15] << 1;
  output[5] += output[15];
  output[4] += output[14] << 4;
  output[4] += output[14] << 1;
  output[4] += output[14];
  output[3] += output[13] << 4;
  output[3] += output[13] << 1;
  output[3] += output[13];
  output[2] += output[12] << 4;
  output[2] += output[12] << 1;
  output[2] += output[12];
  output[1] += output[11] << 4;
  output[1] += output[11] << 1;
  output[1] += output[11];
  output[0] += output[10] << 4;
  output[0] += output[10] << 1;
  output[0] += output[10];
}

#if (-1 & 3) != 3
#error "This code only works on a two's complement system"
#endif

/* return v / 2^26, using only shifts and adds.
 *
 * On entry: v can take any value. */
static inline limb
div_by_2_26(const limb v)
{
  /* High word of v; no shift needed. */
  const uint32_t highword = (uint32_t) (((uint64_t) v) >> 32);
  /* Set to all 1s if v was negative; else set to 0s. */
  const int32_t sign = ((int32_t) highword) >> 31;
  /* Set to 0x3ffffff if v was negative; else set to 0. */
  const int32_t roundoff = ((uint32_t) sign) >> 6;
  /* Should return v / (1<<26) */
  return (v + roundoff) >> 26;
}

/* return v / (2^25), using only shifts and adds.
 *
 * On entry: v can take any value. */
static inline limb
div_by_2_25(const limb v)
{
  /* High word of v; no shift needed*/
  const uint32_t highword = (uint32_t) (((uint64_t) v) >> 32);
  /* Set to all 1s if v was negative; else set to 0s. */
  const int32_t sign = ((int32_t) highword) >> 31;
  /* Set to 0x1ffffff if v was negative; else set to 0. */
  const int32_t roundoff = ((uint32_t) sign) >> 7;
  /* Should return v / (1<<25) */
  return (v + roundoff) >> 25;
}

/* Reduce all coefficients of the short form input so that |x| < 2^26.
 *
 * On entry: |output[i]| < 280*2^54 */
static void freduce_coefficients(limb *output) {
  unsigned i;

  output[10] = 0;

  for (i = 0; i < 10; i += 2) {
    limb over = div_by_2_26(output[i]);
    /* The entry condition (that |output[i]| < 280*2^54) means that over is, at
     * most, 280*2^28 in the first iteration of this loop. This is added to the
     * next limb and we can approximate the resulting bound of that limb by
     * 281*2^54. */
    output[i] -= over << 26;
    output[i+1] += over;

    /* For the first iteration, |output[i+1]| < 281*2^54, thus |over| <
     * 281*2^29. When this is added to the next limb, the resulting bound can
     * be approximated as 281*2^54.
     *
     * For subsequent iterations of the loop, 281*2^54 remains a conservative
     * bound and no overflow occurs. */
    over = div_by_2_25(output[i+1]);
    output[i+1] -= over << 25;
    output[i+2] += over;
  }
  /* Now |output[10]| < 281*2^29 and all other coefficients are reduced. */
  output[0] += output[10] << 4;
  output[0] += output[10] << 1;
  output[0] += output[10];

  output[10] = 0;

  /* Now output[1..9] are reduced, and |output[0]| < 2^26 + 19*281*2^29
   * So |over| will be no more than 2^16. */
  {
    limb over = div_by_2_26(output[0]);
    output[0] -= over << 26;
    output[1] += over;
  }

  /* Now output[0,2..9] are reduced, and |output[1]| < 2^25 + 2^16 < 2^26. The
   * bound on |output[1]| is sufficient to meet our needs. */
}

/* A helpful wrapper around fproduct: output = in * in2.
 *
 * On entry: |in[i]| < 2^27 and |in2[i]| < 2^27.
 *
 * output must be distinct to both inputs. The output is reduced degree
 * (indeed, one need only provide storage for 10 limbs) and |output[i]| < 2^26. */
static void
fmul(limb *output, const limb *in, const limb *in2) {
  limb t[19];
  fproduct(t, in, in2);
  /* |t[i]| < 14*2^54 */
  freduce_degree(t);
  freduce_coefficients(t);
  /* |t[i]| < 2^26 */
  memcpy(output, t, sizeof(limb) * 10);
}

/* Square a number: output = in**2
 *
 * output must be distinct from the input. The inputs are reduced coefficient
 * form, the output is not.
 *
 * output[x] <= 14 * the largest product of the input limbs. */
static void fsquare_inner(limb *output, const limb *in) {
  output[0] =       ((limb) ((s32) in[0])) * ((s32) in[0]);
  output[1] =  2 *  ((limb) ((s32) in[0])) * ((s32) in[1]);
  output[2] =  2 * (((limb) ((s32) in[1])) * ((s32) in[1]) +
                    ((limb) ((s32) in[0])) * ((s32) in[2]));
  output[3] =  2 * (((limb) ((s32) in[1])) * ((s32) in[2]) +
                    ((limb) ((s32) in[0])) * ((s32) in[3]));
  output[4] =       ((limb) ((s32) in[2])) * ((s32) in[2]) +
               4 *  ((limb) ((s32) in[1])) * ((s32) in[3]) +
               2 *  ((limb) ((s32) in[0])) * ((s32) in[4]);
  output[5] =  2 * (((limb) ((s32) in[2])) * ((s32) in[3]) +
                    ((limb) ((s32) in[1])) * ((s32) in[4]) +
                    ((limb) ((s32) in[0])) * ((s32) in[5]));
  output[6] =  2 * (((limb) ((s32) in[3])) * ((s32) in[3]) +
                    ((limb) ((s32) in[2])) * ((s32) in[4]) +
                    ((limb) ((s32) in[0])) * ((s32) in[6]) +
               2 *  ((limb) ((s32) in[1])) * ((s32) in[5]));
  output[7] =  2 * (((limb) ((s32) in[3])) * ((s32) in[4]) +
                    ((limb) ((s32) in[2])) * ((s32) in[5]) +
                    ((limb) ((s32) in[1])) * ((s32) in[6]) +
                    ((limb) ((s32) in[0])) * ((s32) in[7]));
  output[8] =       ((limb) ((s32) in[4])) * ((s32) in[4]) +
               2 * (((limb) ((s32) in[2])) * ((s32) in[6]) +
                    ((limb) ((s32) in[0])) * ((s32) in[8]) +
               2 * (((limb) ((s32) in[1])) * ((s32) in[7]) +
                    ((limb) ((s32) in[3])) * ((s32) in[5])));
  output[9] =  2 * (((limb) ((s32) in[4])) * ((s32) in[5]) +
                    ((limb) ((s32) in[3])) * ((s32) in[6]) +
                    ((limb) ((s32) in[2])) * ((s32) in[7]) +
                    ((limb) ((s32) in[1])) * ((s32) in[8]) +
                    ((limb) ((s32) in[0])) * ((s32) in[9]));
  output[10] = 2 * (((limb) ((s32) in[5])) * ((s32) in[5]) +
                    ((limb) ((s32) in[4])) * ((s32) in[6]) +
                    ((limb) ((s32) in[2])) * ((s32) in[8]) +
               2 * (((limb) ((s32) in[3])) * ((s32) in[7]) +
                    ((limb) ((s32) in[1])) * ((s32) in[9])));
  output[11] = 2 * (((limb) ((s32) in[5])) * ((s32) in[6]) +
                    ((limb) ((s32) in[4])) * ((s32) in[7]) +
                    ((limb) ((s32) in[3])) * ((s32) in[8]) +
                    ((limb) ((s32) in[2])) * ((s32) in[9]));
  output[12] =      ((limb) ((s32) in[6])) * ((s32) in[6]) +
               2 * (((limb) ((s32) in[4])) * ((s32) in[8]) +
               2 * (((limb) ((s32) in[5])) * ((s32) in[7]) +
                    ((limb) ((s32) in[3])) * ((s32) in[9])));
  output[13] = 2 * (((limb) ((s32) in[6])) * ((s32) in[7]) +
                    ((limb) ((s32) in[5])) * ((s32) in[8]) +
                    ((limb) ((s32) in[4])) * ((s32) in[9]));
  output[14] = 2 * (((limb) ((s32) in[7])) * ((s32) in[7]) +
                    ((limb) ((s32) in[6])) * ((s32) in[8]) +
               2 *  ((limb) ((s32) in[5])) * ((s32) in[9]));
  output[15] = 2 * (((limb) ((s32) in[7])) * ((s32) in[8]) +
                    ((limb) ((s32) in[6])) * ((s32) in[9]));
  output[16] =      ((limb) ((s32) in[8])) * ((s32) in[8]) +
               4 *  ((limb) ((s32) in[7])) * ((s32) in[9]);
  output[17] = 2 *  ((limb) ((s32) in[8])) * ((s32) in[9]);
  output[18] = 2 *  ((limb) ((s32) in[9])) * ((s32) in[9]);
}

/* fsquare sets output = in^2.
 *
 * On entry: The |in| argument is in reduced coefficients form and |in[i]| <
 * 2^27.
 *
 * On exit: The |output| argument is in reduced coefficients form (indeed, one
 * need only provide storage for 10 limbs) and |out[i]| < 2^26. */
static void
fsquare(limb *output, const limb *in) {
  limb t[19];
  fsquare_inner(t, in);
  /* |t[i]| < 14*2^54 because the largest product of two limbs will be <
   * 2^(27+27) and fsquare_inner adds together, at most, 14 of those
   * products. */
  freduce_degree(t);
  freduce_coefficients(t);
  /* |t[i]| < 2^26 */
  memcpy(output, t, sizeof(limb) * 10);
}

/* Take a little-endian, 32-byte number and expand it into polynomial form */
static void
fexpand(limb *output, const u8 *input) {
#define F(n,start,shift,mask) \
  output[n] = ((((limb) input[start + 0]) | \
                ((limb) input[start + 1]) << 8 | \
                ((limb) input[start + 2]) << 16 | \
                ((limb) input[start + 3]) << 24) >> shift) & mask;
  F(0, 0, 0, 0x3ffffff);
  F(1, 3, 2, 0x1ffffff);
  F(2, 6, 3, 0x3ffffff);
  F(3, 9, 5, 0x1ffffff);
  F(4, 12, 6, 0x3ffffff);
  F(5, 16, 0, 0x1ffffff);
  F(6, 19, 1, 0x3ffffff);
  F(7, 22, 3, 0x1ffffff);
  F(8, 25, 4, 0x3ffffff);
  F(9, 28, 6, 0x1ffffff);
#undef F
}

#if (-32 >> 1) != -16
#error "This code only works when >> does sign-extension on negative numbers"
#endif

/* s32_eq returns 0xffffffff iff a == b and zero otherwise. */
static s32 s32_eq(s32 a, s32 b) {
  a = ~(a ^ b);
  a &= a << 16;
  a &= a << 8;
  a &= a << 4;
  a &= a << 2;
  a &= a << 1;
  return a >> 31;
}

/* s32_gte returns 0xffffffff if a >= b and zero otherwise, where a and b are
 * both non-negative. */
static s32 s32_gte(s32 a, s32 b) {
  a -= b;
  /* a >= 0 iff a >= b. */
  return ~(a >> 31);
}

/* Take a fully reduced polynomial form number and contract it into a
 * little-endian, 32-byte array.
 *
 * On entry: |input_limbs[i]| < 2^26 */
static void
fcontract(u8 *output, limb *input_limbs) {
  int i;
  int j;
  s32 input[10];
  s32 mask;

  /* |input_limbs[i]| < 2^26, so it's valid to convert to an s32. */
  for (i = 0; i < 10; i++) {
    input[i] = input_limbs[i];
  }

  for (j = 0; j < 2; ++j) {
    for (i = 0; i < 9; ++i) {
      if ((i & 1) == 1) {
        /* This calculation is a time-invariant way to make input[i]
         * non-negative by borrowing from the next-larger limb. */
        const s32 mask = input[i] >> 31;
        const s32 carry = -((input[i] & mask) >> 25);
        input[i] = input[i] + (carry << 25);
        input[i+1] = input[i+1] - carry;
      } else {
        const s32 mask = input[i] >> 31;
        const s32 carry = -((input[i] & mask) >> 26);
        input[i] = input[i] + (carry << 26);
        input[i+1] = input[i+1] - carry;
      }
    }

    /* There's no greater limb for input[9] to borrow from, but we can multiply
     * by 19 and borrow from input[0], which is valid mod 2^255-19. */
    {
      const s32 mask = input[9] >> 31;
      const s32 carry = -((input[9] & mask) >> 25);
      input[9] = input[9] + (carry << 25);
      input[0] = input[0] - (carry * 19);
    }

    /* After the first iteration, input[1..9] are non-negative and fit within
     * 25 or 26 bits, depending on position. However, input[0] may be
     * negative. */
  }

  /* The first borrow-propagation pass above ended with every limb
     except (possibly) input[0] non-negative.

     If input[0] was negative after the first pass, then it was because of a
     carry from input[9]. On entry, input[9] < 2^26 so the carry was, at most,
     one, since (2**26-1) >> 25 = 1. Thus input[0] >= -19.

     In the second pass, each limb is decreased by at most one. Thus the second
     borrow-propagation pass could only have wrapped around to decrease
     input[0] again if the first pass left input[0] negative *and* input[1]
     through input[9] were all zero.  In that case, input[1] is now 2^25 - 1,
     and this last borrow-propagation step will leave input[1] non-negative. */
  {
    const s32 mask = input[0] >> 31;
    const s32 carry = -((input[0] & mask) >> 26);
    input[0] = input[0] + (carry << 26);
    input[1] = input[1] - carry;
  }

  /* All input[i] are now non-negative. However, there might be values between
   * 2^25 and 2^26 in a limb which is, nominally, 25 bits wide. */
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 9; i++) {
      if ((i & 1) == 1) {
        const s32 carry = input[i] >> 25;
        input[i] &= 0x1ffffff;
        input[i+1] += carry;
      } else {
        const s32 carry = input[i] >> 26;
        input[i] &= 0x3ffffff;
        input[i+1] += carry;
      }
    }

    {
      const s32 carry = input[9] >> 25;
      input[9] &= 0x1ffffff;
      input[0] += 19*carry;
    }
  }

  /* If the first carry-chain pass, just above, ended up with a carry from
   * input[9], and that caused input[0] to be out-of-bounds, then input[0] was
   * < 2^26 + 2*19, because the carry was, at most, two.
   *
   * If the second pass carried from input[9] again then input[0] is < 2*19 and
   * the input[9] -> input[0] carry didn't push input[0] out of bounds. */

  /* It still remains the case that input might be between 2^255-19 and 2^255.
   * In this case, input[1..9] must take their maximum value and input[0] must
   * be >= (2^255-19) & 0x3ffffff, which is 0x3ffffed. */
  mask = s32_gte(input[0], 0x3ffffed);
  for (i = 1; i < 10; i++) {
    if ((i & 1) == 1) {
      mask &= s32_eq(input[i], 0x1ffffff);
    } else {
      mask &= s32_eq(input[i], 0x3ffffff);
    }
  }

  /* mask is either 0xffffffff (if input >= 2^255-19) and zero otherwise. Thus
   * this conditionally subtracts 2^255-19. */
  input[0] -= mask & 0x3ffffed;

  for (i = 1; i < 10; i++) {
    if ((i & 1) == 1) {
      input[i] -= mask & 0x1ffffff;
    } else {
      input[i] -= mask & 0x3ffffff;
    }
  }

  input[1] <<= 2;
  input[2] <<= 3;
  input[3] <<= 5;
  input[4] <<= 6;
  input[6] <<= 1;
  input[7] <<= 3;
  input[8] <<= 4;
  input[9] <<= 6;
#define F(i, s) \
  output[s+0] |=  input[i] & 0xff; \
  output[s+1]  = (input[i] >> 8) & 0xff; \
  output[s+2]  = (input[i] >> 16) & 0xff; \
  output[s+3]  = (input[i] >> 24) & 0xff;
  output[0] = 0;
  output[16] = 0;
  F(0,0);
  F(1,3);
  F(2,6);
  F(3,9);
  F(4,12);
  F(5,16);
  F(6,19);
  F(7,22);
  F(8,25);
  F(9,28);
#undef F
}

/* Input: Q, Q', Q-Q'
 * Output: 2Q, Q+Q'
 *
 *   x2 z3: long form
 *   x3 z3: long form
 *   x z: short form, destroyed
 *   xprime zprime: short form, destroyed
 *   qmqp: short form, preserved
 *
 * On entry and exit, the absolute value of the limbs of all inputs and outputs
 * are < 2^26. */
static void fmonty(limb *x2, limb *z2,  /* output 2Q */
                   limb *x3, limb *z3,  /* output Q + Q' */
                   limb *x, limb *z,    /* input Q */
                   limb *xprime, limb *zprime,  /* input Q' */
                   const limb *qmqp /* input Q - Q' */) {
  limb origx[10], origxprime[10], zzz[19], xx[19], zz[19], xxprime[19],
        zzprime[19], zzzprime[19], xxxprime[19];

  memcpy(origx, x, 10 * sizeof(limb));
  fsum(x, z);
  /* |x[i]| < 2^27 */
  fdifference(z, origx);  /* does x - z */
  /* |z[i]| < 2^27 */

  memcpy(origxprime, xprime, sizeof(limb) * 10);
  fsum(xprime, zprime);
  /* |xprime[i]| < 2^27 */
  fdifference(zprime, origxprime);
  /* |zprime[i]| < 2^27 */
  fproduct(xxprime, xprime, z);
  /* |xxprime[i]| < 14*2^54: the largest product of two limbs will be <
   * 2^(27+27) and fproduct adds together, at most, 14 of those products.
   * (Approximating that to 2^58 doesn't work out.) */
  fproduct(zzprime, x, zprime);
  /* |zzprime[i]| < 14*2^54 */
  freduce_degree(xxprime);
  freduce_coefficients(xxprime);
  /* |xxprime[i]| < 2^26 */
  freduce_degree(zzprime);
  freduce_coefficients(zzprime);
  /* |zzprime[i]| < 2^26 */
  memcpy(origxprime, xxprime, sizeof(limb) * 10);
  fsum(xxprime, zzprime);
  /* |xxprime[i]| < 2^27 */
  fdifference(zzprime, origxprime);
  /* |zzprime[i]| < 2^27 */
  fsquare(xxxprime, xxprime);
  /* |xxxprime[i]| < 2^26 */
  fsquare(zzzprime, zzprime);
  /* |zzzprime[i]| < 2^26 */
  fproduct(zzprime, zzzprime, qmqp);
  /* |zzprime[i]| < 14*2^52 */
  freduce_degree(zzprime);
  freduce_coefficients(zzprime);
  /* |zzprime[i]| < 2^26 */
  memcpy(x3, xxxprime, sizeof(limb) * 10);
  memcpy(z3, zzprime, sizeof(limb) * 10);

  fsquare(xx, x);
  /* |xx[i]| < 2^26 */
  fsquare(zz, z);
  /* |zz[i]| < 2^26 */
  fproduct(x2, xx, zz);
  /* |x2[i]| < 14*2^52 */
  freduce_degree(x2);
  freduce_coefficients(x2);
  /* |x2[i]| < 2^26 */
  fdifference(zz, xx);  // does zz = xx - zz
  /* |zz[i]| < 2^27 */
  memset(zzz + 10, 0, sizeof(limb) * 9);
  fscalar_product(zzz, zz, 121665);
  /* |zzz[i]| < 2^(27+17) */
  /* No need to call freduce_degree here:
     fscalar_product doesn't increase the degree of its input. */
  freduce_coefficients(zzz);
  /* |zzz[i]| < 2^26 */
  fsum(zzz, xx);
  /* |zzz[i]| < 2^27 */
  fproduct(z2, zz, zzz);
  /* |z2[i]| < 14*2^(26+27) */
  freduce_degree(z2);
  freduce_coefficients(z2);
  /* |z2|i| < 2^26 */
}

/* Conditionally swap two reduced-form limb arrays if 'iswap' is 1, but leave
 * them unchanged if 'iswap' is 0.  Runs in data-invariant time to avoid
 * side-channel attacks.
 *
 * NOTE that this function requires that 'iswap' be 1 or 0; other values give
 * wrong results.  Also, the two limb arrays must be in reduced-coefficient,
 * reduced-degree form: the values in a[10..19] or b[10..19] aren't swapped,
 * and all all values in a[0..9],b[0..9] must have magnitude less than
 * INT32_MAX. */
static void
swap_conditional(limb a[19], limb b[19], limb iswap) {
  unsigned i;
  const s32 swap = (s32) -iswap;

  for (i = 0; i < 10; ++i) {
    const s32 x = swap & ( ((s32)a[i]) ^ ((s32)b[i]) );
    a[i] = ((s32)a[i]) ^ x;
    b[i] = ((s32)b[i]) ^ x;
  }
}

/* Calculates nQ where Q is the x-coordinate of a point on the curve
 *
 *   resultx/resultz: the x coordinate of the resulting curve point (short form)
 *   n: a little endian, 32-byte number
 *   q: a point of the curve (short form) */
static void
cmult(limb *resultx, limb *resultz, const u8 *n, const limb *q) {
  limb a[19] = {0}, b[19] = {1}, c[19] = {1}, d[19] = {0};
  limb *nqpqx = a, *nqpqz = b, *nqx = c, *nqz = d, *t;
  limb e[19] = {0}, f[19] = {1}, g[19] = {0}, h[19] = {1};
  limb *nqpqx2 = e, *nqpqz2 = f, *nqx2 = g, *nqz2 = h;

  unsigned i, j;

  memcpy(nqpqx, q, sizeof(limb) * 10);

  for (i = 0; i < 32; ++i) {
    u8 byte = n[31 - i];
    for (j = 0; j < 8; ++j) {
      const limb bit = byte >> 7;

      swap_conditional(nqx, nqpqx, bit);
      swap_conditional(nqz, nqpqz, bit);
      fmonty(nqx2, nqz2,
             nqpqx2, nqpqz2,
             nqx, nqz,
             nqpqx, nqpqz,
             q);
      swap_conditional(nqx2, nqpqx2, bit);
      swap_conditional(nqz2, nqpqz2, bit);

      t = nqx;
      nqx = nqx2;
      nqx2 = t;
      t = nqz;
      nqz = nqz2;
      nqz2 = t;
      t = nqpqx;
      nqpqx = nqpqx2;
      nqpqx2 = t;
      t = nqpqz;
      nqpqz = nqpqz2;
      nqpqz2 = t;

      byte <<= 1;
    }
  }

  memcpy(resultx, nqx, sizeof(limb) * 10);
  memcpy(resultz, nqz, sizeof(limb) * 10);
}

// -----------------------------------------------------------------------------
// Shamelessly copied from djb's code
// -----------------------------------------------------------------------------
static void
crecip(limb *out, const limb *z) {
  limb z2[10];
  limb z9[10];
  limb z11[10];
  limb z2_5_0[10];
  limb z2_10_0[10];
  limb z2_20_0[10];
  limb z2_50_0[10];
  limb z2_100_0[10];
  limb t0[10];
  limb t1[10];
  int i;

  /* 2 */ fsquare(z2,z);
  /* 4 */ fsquare(t1,z2);
  /* 8 */ fsquare(t0,t1);
  /* 9 */ fmul(z9,t0,z);
  /* 11 */ fmul(z11,z9,z2);
  /* 22 */ fsquare(t0,z11);
  /* 2^5 - 2^0 = 31 */ fmul(z2_5_0,t0,z9);

  /* 2^6 - 2^1 */ fsquare(t0,z2_5_0);
  /* 2^7 - 2^2 */ fsquare(t1,t0);
  /* 2^8 - 2^3 */ fsquare(t0,t1);
  /* 2^9 - 2^4 */ fsquare(t1,t0);
  /* 2^10 - 2^5 */ fsquare(t0,t1);
  /* 2^10 - 2^0 */ fmul(z2_10_0,t0,z2_5_0);

  /* 2^11 - 2^1 */ fsquare(t0,z2_10_0);
  /* 2^12 - 2^2 */ fsquare(t1,t0);
  /* 2^20 - 2^10 */ for (i = 2;i < 10;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^20 - 2^0 */ fmul(z2_20_0,t1,z2_10_0);

  /* 2^21 - 2^1 */ fsquare(t0,z2_20_0);
  /* 2^22 - 2^2 */ fsquare(t1,t0);
  /* 2^40 - 2^20 */ for (i = 2;i < 20;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^40 - 2^0 */ fmul(t0,t1,z2_20_0);

  /* 2^41 - 2^1 */ fsquare(t1,t0);
  /* 2^42 - 2^2 */ fsquare(t0,t1);
  /* 2^50 - 2^10 */ for (i = 2;i < 10;i += 2) { fsquare(t1,t0); fsquare(t0,t1); }
  /* 2^50 - 2^0 */ fmul(z2_50_0,t0,z2_10_0);

  /* 2^51 - 2^1 */ fsquare(t0,z2_50_0);
  /* 2^52 - 2^2 */ fsquare(t1,t0);
  /* 2^100 - 2^50 */ for (i = 2;i < 50;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^100 - 2^0 */ fmul(z2_100_0,t1,z2_50_0);

  /* 2^101 - 2^1 */ fsquare(t1,z2_100_0);
  /* 2^102 - 2^2 */ fsquare(t0,t1);
  /* 2^200 - 2^100 */ for (i = 2;i < 100;i += 2) { fsquare(t1,t0); fsquare(t0,t1); }
  /* 2^200 - 2^0 */ fmul(t1,t0,z2_100_0);

  /* 2^201 - 2^1 */ fsquare(t0,t1);
  /* 2^202 - 2^2 */ fsquare(t1,t0);
  /* 2^250 - 2^50 */ for (i = 2;i < 50;i += 2) { fsquare(t0,t1); fsquare(t1,t0); }
  /* 2^250 - 2^0 */ fmul(t0,t1,z2_50_0);

  /* 2^251 - 2^1 */ fsquare(t1,t0);
  /* 2^252 - 2^2 */ fsquare(t0,t1);
  /* 2^253 - 2^3 */ fsquare(t1,t0);
  /* 2^254 - 2^4 */ fsquare(t0,t1);
  /* 2^255 - 2^5 */ fsquare(t1,t0);
  /* 2^255 - 21 */ fmul(out,t1,z11);
}

int
curve25519_donna_ref(u8 *mypublic, const u8 *secret, const u8 *basepoint) {
  limb bp[10], x[10], z[11], zmone[10];
  uint8_t e[32];
  int i;

  for (i = 0; i < 32; ++i) e[i] = secret[i];
  e[0] &= 248;
  e[31] &= 127;
  e[31] |= 64;

  fexpand(bp, basepoint);
  cmult(x, z, e, bp);
  crecip(zmone, z);
  fmul(z, x, zmone);
  fcontract(mypublic, z);
  return 0;
}