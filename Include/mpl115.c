#include "mpl115.h"
#include <delay.h>
#include "i2c.h"
#include <stdio.h>

void mpl115_convert() 
{
	int pressure = 0;
	int temperature = 0;
	char buffer[16];

/*
Note (Write/Read) W = 0, R = 1
7bit address = 0x60
Command to Write “Convert both Pressure and Temperature” = 0x12
Command to Read “Pressure High Byte” = 0x00
Command to Read “Coefficient data byte 1 High byte” = 0x04
Start Pressure and Temperature Conversion, Read raw Pressure:
[Start], 0x60+[W], 0x12, 0x01,[Stop]
[Restart], 0x60+[W], 0x00
[Start], 0x60+[R], 0xMSB Pressure, 0xLSB Pressure, [Stop]
*/

	i2c_start();
	i2c_write(MPL115_ADDRESS);
	i2c_write(MPL115_CONVERT_BOTH);
	i2c_write(0x01);
	i2c_stop();

	// Wait for the conversion to complete.  Datasheet says
	// max 1ms.
	DelayMs(1);

	i2c_restart();
	i2c_write(MPL115_ADDRESS);
	i2c_write(MPL115_READ_PRESSURE);

	i2c_start();
	i2c_write(MPL115_ADDRESS | 1);

/*
	pressure = i2c_read(I2C_ACK);
	pressure <<= 8;
	pressure |= i2c_read(I2C_ACK);

	temperature = i2c_read(I2C_ACK);
	temperature <<= 8;
	temperature |= i2c_read(I2C_NAK);
*/

	for (char i=0; i<15; i++) {	
		buffer[i] = i2c_read(I2C_ACK);
	}
	buffer[15] = i2c_read(I2C_NAK);

	i2c_stop();

	unsigned int pout = buffer[0] << 2 | buffer[1] >> 6;
	unsigned int tout = buffer[2] << 2 | buffer[3] >> 6;

//	unsigned int pout = 

        
	printf("MPL115 pressure was: %u\r\n", pout);
	printf("MPL115 temperature was: %u\r\n", tout);

        /*
	printf("Raw values were: ");
	for (int i=0; i<16; i++) {
		printf("%02x ", buffer[i]);
	}
	printf("\r\n");
        */

// Pcomp = a0 + (b1 + c11*Padc + c12*Tadc) * Padc + (b2 + c22*Tadc) * Tadc
/*
Where:
Padc is the 10-bit pressure output of the MPL115A2 ADC,
Tadc is the 10-bit temperature output of the MPL115A2 ADC,
a0 is the pressure offset coefficient,
b1 is the pressure sensitivity coefficient,
c11 is the pressure linearity (2nd order) coefficient,
c12 is the coefficient for temperature sensitivity coefficient (TCS),
b2 is the 1st order temperature offset coefficient (TCO),
c22 is the 2nd order temperature offset coefficient.
*/

}