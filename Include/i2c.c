#include <htc.h>
#include "i2c.h"

// setup the MSSP module for i2c master mode.
// requires TRIS to be input for the pins
void i2c_init()
{
	// Configure MSSP1 for I2C, used by DS1337 RTC
	SSP1CON1bits.SSPEN = 1;

	// Setup SSPxM for mode, I2C master using baud rate generator
	SSP1CON1bits.SSPM = 0b1000;

	// Setup the MSSP baud rate generator
	// Fosc of 16Mhz, I2C speed 100Khz (table 15-4 datasheet p254)
	SSP1ADD = 0x27;

	return;
}


void i2c_start()
{
	SSP1CON2bits.SEN = 1;
	while(SSP1CON2bits.SEN);

	return;
}

void i2c_restart()
{
	SSP1CON2bits.RSEN = 1;
	while(SSP1CON2bits.RSEN);

	return;
}


void i2c_stop()
{
	SSP1CON2bits.PEN = 1;
	while(SSP1CON2bits.PEN);

	return;
}

char i2c_write(char data)
{
	SSP1BUF = data;
	while(SSP1STATbits.RW);

	return SSP1CON2bits.ACKSTAT;
}

char i2c_read(char ack)
{
	SSP1CON2bits.RCEN = 1;

	SSP1CON2bits.ACKDT = ack;

// ACKEN + ACKDT

	while(!SSP1STATbits.BF);

	SSP1CON2bits.RCEN = 0;

	SSP1CON2bits.ACKEN = 1;
	while(SSP1CON2bits.ACKEN);

	return SSP1BUF;
}

void i2c_resync()
{
	// Toggle SCL until SDA is observed to be high
	// SCL = RC3
	// SDA = RC4
	while(!PORTCbits.RC4) {
		LATCbits.LATC3 = 1;
		LATCbits.LATC3 = 0;
	}

	LATCbits.LATC3 = 1;
}