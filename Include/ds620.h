/* ds620.h 
 *
 * Author: David <david@edeca.net>
 *
 * Header for DS620 temperature sensor library
 */

 /* 
  * User definable settings
  */

// N/A
  
 /*
  * Constants, do not edit below this line
  */

// I2C address, format:
//   0b1001        - fixed
//         AAA     - 3 address bits
//            x    - I2C R/W bit
#define DS620_CONTROL_BYTE 0b10010000

// DS620 commands
#define DS620_START_CONVERT 0b01010001

// Internal memory locations
#define DS620_TEMP_MSB 0xAA
#define DS620_TEMP_LSB 0xAB
#define DS620_CONFIG_MSB 0xAC
#define DS620_CONFIG_LSB 0xAD

/*
 * Prototypes
 */
void ds620_PrintTemperature(short);
signed short ds620_GetDecimal(short)
unsigned short ds620_GetTemperature(int);