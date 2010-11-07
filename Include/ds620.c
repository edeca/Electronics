/* ds620.c
 *
 *    Author: David <david@edeca.net>
 *      Date: November 2010
 * Datasheet: http://datasheets.maxim-ic.com/en/ds/DS620.pdf
 *   License: BSD (see BSD-LICENSE.TXT)
 *  Compiler: Designed for HI-TECH PICC
 *
 * TODO:
 *   - allow setting of configuration register
 *   - implement continuous conversion mode
 *   - allow setting PO (thermostat) pins
 *
 * Nov 06 2010 - initial version
 */

#include "ds620.h"

// Print the current temperature using printf
//
// Warning: this takes huge amounts of memory
void
ds620_PrintTemperature(short reading)
{
	//int fraction[] = { 

	// Print the decimal portion
	printf("%d.\r\n", reading >> 7);
}

// Return the whole number part
signed short
ds620_GetDecimal(short reading)
{
	signed short decimal;
	
	// The DS620 returns 9-bit 2's complement, so sign extend
	// it here to 16 bits.
	// m = 1U << (b - 1); r = -(x & m) | x
	reading >>= 7;
	decimal = -(reading & 256) | reading;
	
	return decimal;
}

// Get the current temperature from the DS620
//
// Return is 16 bit short:
//   0b
//     S                 Sign
//      DDDDDDDD         Decimal (2^7 to 2^0)
//              FFF      Fraction (2^-1 to 2^-4)
//                 000   Hard wired to 0
//
unsigned short
ds620_GetTemperature(int address)
{
	unsigned short temperature;

	// Lowest three bits are the address, shift left one
	// to allow for I2C R/W bit
	address &= 0b00000111;
	address <<= 1;
	address |= DS620_CONTROL_BYTE;
	
	// Send the start conversion command (one-shot mode)
	i2c_Start();
	i2c_WriteTo(address);
	i2c_PutByte(DS620_START_CONVERT);
	i2c_Stop();

	// TODO: How long do we really need to wait here?
	DelayMs(50);

	// Now start reading from the MSB register
	i2c_Start();
	i2c_WriteTo(address);
	i2c_PutByte(DS620_TEMP_MSB);

	// i2c bus "restart", switch into read mode
	i2c_ReadFrom(address);

	// Read 2 bytes, the MSB then LSB
	temperature = i2c_GetByte(I2C_MORE) << 8;
	temperature |= i2c_GetByte(I2C_MORE);

	i2c_Stop();
		
	return temperature;
}
