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
#include "i2c.h"
#include "delay.h"

int 
_ds620_GetI2CAddress(int address)
{
	// Lowest three bits are the address, shift left one
	// to allow for I2C R/W bit
	address &= 0b00000111;
	address <<= 1;
	address |= DS620_ADDRESS_MASK;
	return address;
}

void
ds620_PrintTemperature(short reading)
{
	//int fraction[] = { 

	// Print the decimal portion
	//printf("%d.\r\n", reading >> 7);
}

void 
ds620_CopyData(int address)
{
	// Copy data from SRAM to EEPROM
	i2c_Start();
	i2c_WriteTo(_ds620_GetI2CAddress(address));

	/**
	 * @todo We must be in continuous conversion mode for this to work, so backup the config register, switch to continuous mode then restore after.
     */

	// Generate another start condition & send address
	// Switch to continuous conversion mode

	// Generate another start condition & send address
	// Send start convert command

	// Generate another start condition & send address
	// Put back in 1-shot mode (if required)

	// Generate another start condition & send address
	// Send COPY DATA command
	i2c_PutByte(DS620_COPY_DATA);

	// Wait 10ms (or check NVB in config register?)

	// Generate another start condition & send address
	// Stop conversion

	// Stop i2c communication
	i2c_Stop();
}

signed short
ds620_ToDecimal(short reading)
{
	signed short decimal;
	
	// The DS620 returns 9-bit 2's complement, so sign extend
	// it here to 16 bits.
	// m = 1U << (b - 1); r = -(x & m) | x
	reading >>= 7;
	decimal = -(reading & 256) | reading;
	
	return decimal;
}

void 
ds620_WriteRegister16(int address, int reg, unsigned short data)
{
	unsigned short value;

	// Load the DS620 with the correct register address and write
	// 2 bytes, MSB first.
	i2c_Start();
	i2c_WriteTo(_ds620_GetI2CAddress(address));
	i2c_PutByte(reg);
	i2c_PutByte((int)data >> 8);
	i2c_PutByte((int)data & 0xFF);
	i2c_Stop();
}

unsigned short
ds620_ReadRegister16(int address, int reg) 
{
	unsigned short value;
	
	// Now start reading from the MSB register
	i2c_Start();
	i2c_WriteTo(_ds620_GetI2CAddress(address));
	i2c_PutByte(reg);

	// i2c bus "restart", switch into read mode
	i2c_ReadFrom(address);

	// Read 2 bytes, the MSB then LSB
	value = i2c_GetByte(I2C_MORE) << 8;
	value |= i2c_GetByte(I2C_MORE);

	i2c_Stop();
		
	return value;
}

// Get the current temperature from the DS620
//
// Return is 16 bit short:
//   0b
//     S                 Sign
//      DDDDDDDD         Decimal (2^7 to 2^0)
//              FFFF     Fraction (2^-1 to 2^-4)
//                  000  Hard wired to 0
//
unsigned short
ds620_GetTemperature(int address)
{
	// Send the start conversion command (one-shot mode)
	i2c_Start();
	i2c_WriteTo(_ds620_GetI2CAddress(address));
	i2c_PutByte(DS620_START_CONVERT);
	i2c_Stop();

	// TODO: How long do we really need to wait here?
	DelayMs(50);

	return ds620_ReadRegister16(address, DS620_TEMP_MSB);	
}

ds620_config 
ds620_GetConfiguration(int address)
{
	ds620_config r;
	r.value = ds620_ReadRegister16(address, DS620_CONFIG_MSB);
	return r;
}
