/*
    CRC-8 routines

        Adapted from code found at:

        http://www.dattalo.com/technical/software/pic/crc_8bit.c

    From Maxim/Dallas AP Note 27

    "Understanding and Using Cyclic Redundancy Checks with 
    Dallas Semiconductor iButton Products"

    The Ap note describes the CRC-8 algorithm used in the 
    iButton products. Their implementation involves a 256 byte
    CRC table. This algorithm is implemented here. In addition
    two other algorithms are shown. One uses nibble arrays and
    the other uses boolean arithmetic.

    18 January 03   - T. Scott Dattalo
        26 July 10	    - David Cannings
 */

#include "crc8.h"

// Global CRC variable
unsigned char crc;

// Lets external code initialise the CRC variable, required if 
// callers use crc8_byte

void crc8_init(unsigned char c) {
    crc = c;
}

#ifdef CRC_MATH
// A basic implementation of CRC8 using only arithmetic

unsigned char crc_math(unsigned char data) {
    int i = (data ^ crc) & 0xff;

    crc = 0;

    if (i & 1)
        crc ^= 0x5e;
    if (i & 2)
        crc ^= 0xbc;
    if (i & 4)
        crc ^= 0x61;
    if (i & 8)
        crc ^= 0xc2;
    if (i & 0x10)
        crc ^= 0x9d;
    if (i & 0x20)
        crc ^= 0x23;
    if (i & 0x40)
        crc ^= 0x46;
    if (i & 0x80)
        crc ^= 0x8c;

    return crc;
}
#endif

#ifdef CRC_TABLE

unsigned char crc_table(unsigned char data) {
    int i = (data ^ crc) & 0xff;

    crc = crc_array[(data ^ crc)&0xff];

    return crc;
}
#endif


#ifdef CRC_NIBBLES

unsigned char crc_nibbles(unsigned char data) {
    unsigned char i = (data ^ crc) & 0xff;

    crc = r1[i & 0xf] ^ r2[i >> 4];

    return crc;
}
#endif

// Calculate the CRC for an individual byte
// Will use the quickest method depending on what #defines
// have been chosen

unsigned char crc8_byte(unsigned char data) {
#ifdef CRC_TABLE
    crc_table(data);
#endif

#ifdef CRC_NIBBLES
    crc_nibbles(data);
#endif

#ifdef CRC_MATH
    crc_math(data);
#endif

    return crc;
}

unsigned char crc8_stream(unsigned char *data, unsigned int length) {
    crc8_init(0xff);

    for (int i = 0; i < length; i++) {
        crc8_byte(data[i]);
    }

    return crc;
}


/*
int main(int argc, char **argv)
{
  int i;

  crc = 0xff;

  /*
    Standard check for the string "123456789"
  
  
  for(i='1'; i<='9'; i++) {
    crc8(i);
    printf(" %d 0x%02x\n",i,crc);
  }
  
  printf("\n----\n\n");

  /* 
     Orthogonal check
     Set the crc =0 and data = 2^n and compute new crc
  

  crc = 0x00;
  crc8(1);
  crc = 0x00;
  crc8(2);
  crc = 0x00;
  crc8(4);
  crc = 0x00;
  crc8(8);
  crc = 0x00;
  crc8(16);
  crc = 0x00;
  crc8(32);
  crc = 0x00;
  crc8(64);
  crc = 0x00;
  crc8(128);

  /*
    CRC example in the Dallas APNote
  
  crc=0;
  i = 0x02;
  printf(" %3x 0x%02x\n",i,crc8(i));
  
  i = 0x1c;
  printf(" %3x 0x%02x\n",i,crc8(i));
  
  i = 0xb8;
  printf(" %3x 0x%02x\n",i,crc8(i));
  
  i = 0x01;
  printf(" %3x 0x%02x\n",i,crc8(i));
  
  i = 0x00;
  printf(" %3x 0x%02x\n",i,crc8(i));
  
  i = 0x00;
  printf(" %3x 0x%02x\n",i,crc8(i));
  
  i = 0x00;
  printf(" %3x 0x%02x\n",i,crc8(i));
  
  i = 0xa2;
  printf(" %3x 0x%02x\n",i,crc8(i));

  return 0;
}*/