#include "user_mmap_driver.h"

#define BBRAM_BASE 0xFFCD0000
#define BBRAM_LENGTH 0x48
#define BBRAM_STATUS 0x0
#define BBRAM_PROGRAM 0x8
//magic value to enable BBRAM programming
#define BBRAM_PROGRAM_MAGIC 0x757BDF0D
#define BBRAM_KEY 0x10


#define KEY_0 = 0x00000000
#define KEY_1 = 0x00000000
#define KEY_2 = 0x00000000
#define KEY_3 = 0x00000000
#define KEY_4 = 0x00000000
#define KEY_5 = 0x00000000
#define KEY_6 = 0x00000000
#define KEY_7 = 0x00000000

unsigned char STATIC_KEY[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

unsigned char KEY_EXTRA[] = {
  0x00, 0x00, 0x00, 0x00
};

typedef union crc_un{
  uint32_t valInt;
  unsigned char bytes[4];
} crc_union;

int main(){
  int i;
  shared_memory bbram_config = getSharedMemoryArea(BBRAM_BASE, BBRAM_LENGTH);
  unsigned char *bbram_reg = (unsigned char*)(bbram_config->ptr);

  //print current BBRAM status
  printf("BBRAM status register: %02x\n", bbram_config[BBRAM_STATUS]);

  //set BBRAM to programming mode
  bbram_reg[BBRAM_PROGRAM] = BBRAM_PROGRAM_MAGIC;

  //print current BBRAM status again to show programmng enabled
  //TODO: need to poll the PGM_MODE bit to see when programming mode is actually
  //enabled. also the BBRAM_ZEROIZED bit
  printf("BBRAM status register: %02x\n", bbram_config[BBRAM_STATUS]);

  //calculate key crc
  crc_union crc_key.valInt = crcSlow(STATIC_KEY, 32);

  //program static key to bbram
  for(i=0; i<32; i++){
    bbram_reg[BBRAM_KEY + i] = STATIC_KEY[i];
  }

  //program key crc to bbram
  for(i=0; i<4; i++){
    bbram_reg[BBRAM_KEY + 32] = crc_key.bytes[i];
  }

  //print current BBRAM status
  //TODO: need to wait for the AES_CRC_DONE bit to be set for crc calculation
  //to complete and for the AES_CRC_PASS bit to be set for crc calculation
  //to be determined as successful
  printf("BBRAM status register: %02x\n", bbram_config[BBRAM_STATUS]);
}

/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 * Source: https://barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
 */
typedef uint32_t crc;

#define WIDTH  (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))

crc
crcSlow(uint8_t const message[], int nBytes)
{
    crc  remainder = 0;


    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (int byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        remainder ^= (message[byte] << (WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    return (remainder);

}   /* crcSlow() */
