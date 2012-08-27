/**
 * Freescale Application Note AN3785 does a much better job of explaining
 * the I2C interface than the datasheet. 
 */

// The pressure sensor address
#define MPL115_ADDRESS 0xC0

#define MPL115_READ_PRESSURE 0x00

#define MPL115_CONVERT_PRESSURE 0b00010000
#define MPL115_CONVERT_TEMPERATURE 0b00010001
#define MPL115_CONVERT_BOTH 0b0010010

//Start Pressure Conversion X0010000 User I2C
//Start Temperature Conversion X0010001 User I2C
//Start both Conversions X0010010 User I2C

void mpl115_convert();