/**
 * @file   ds620.h
 * @author David <david@edeca.net>
 * @date   November, 2010
 * @brief  Header for DS620 temperature sensor library.
 * @details
 *
 * A library for accessing the DS620 temperature sensor, using i2c.
 *
 * Example Usage:
 * @code
 *    unsigned short temp = ds620_GetTemperature(0b001);
 *    ds620_PrintTemperature(temp);
 *    signed short decimal = ds620_ToDecimal(temp);
 * @endcode
 *
 * @todo Add a function to set continuous or one-shot mode
 * @todo Add functions to allow storage/retrieval of data in the user registers
 */

#ifndef _DS620_H
#define _DS620_H

/* 
 * User definable settings
 */

// None
  
/*
 * Constants, do not edit below this line
 */

/** 
 * The I2C address of the DS620 
 *
 *  0b1001        - fixed
 *        AAA     - 3 address bits
 *           x    - I2C R/W bit
 */
#define DS620_ADDRESS_MASK 0b10010000

/** Start converting temperature command */
#define DS620_START_CONVERT 0x51
/** Stop converting temperature command */
#define DS620_STOP_CONVERT 0x22
/** Copy data from EEPROM to SRAM (shadow registers) command */
#define DS620_RECALL_DATA 0xB8
/** Copy data from SRAM (shadow registers) to EEPROM command */
#define DS620_COPY_DATA 0x48

/** @defgroup ds620_addresses DS620 internal memory locations
 *  @{
 */

/** Current temperature (MSB) */
#define DS620_TEMP_MSB 0xAA
/** Current temperature (LSB) */
#define DS620_TEMP_LSB 0xAB
/** Internal configuration register (MSB) */
#define DS620_CONFIG_MSB 0xAC
/** Internal configuration register (LSB) */
#define DS620_CONFIG_LSB 0xAD

/** @} */

/** The configuration register */
typedef union
{
	struct
	{
		unsigned DONE:1;
		unsigned NVB:1;
		unsigned THF:1;
		unsigned TLF:1;
		unsigned R1:1;
		unsigned R0:1;
		unsigned AUTOC:1;
		unsigned ONESHOT:1;
		unsigned PO2:1;
		unsigned PO1:1;
		unsigned A2:1;
		unsigned A1:1;
		unsigned A0:1;
		unsigned USER2:1;
		unsigned USER1:1;
		unsigned USER0:1;
	} bits;
	unsigned short value;
} ds620_config;

/**
 * Print a temperature value using printf
 *
 * @param reading A 16-bit value from the sensor
 */
void ds620_PrintTemperature(short reading);
/**
 * Read a 16-bit register.
 *
 * This function retrieves two adjacent bytes from the
 * DS620, storing the result as a 16 bit value.
 *
 * @param address 	The sensor address (0-7)
 * @param reg		The internal register
 */
unsigned short ds620_ReadRegister16(int address, int reg);
/**
 * Retrieve the decimal portion of the sensor reading.
 *
 * Removes the fraction part and handles sign extension
 * from 9-bit to a 16-bit short.
 *
 * @param reading A 16-bit value from the sensor
 */
signed short ds620_ToDecimal(short reading);
/**
 * Get the current temperature register as a 16-bit value.
 *
 * @param address	The sensor address (0-7)
 */
unsigned short ds620_GetTemperature(int address);
/**
 * Get the internal configuration as a 16-bit value.
 *
 * @param address	The sensor address (0-7)
 */
ds620_config ds620_GetConfiguration(int address);

#endif
