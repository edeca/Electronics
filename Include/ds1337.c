#include "ds1337.h"
#include "i2c.h"

#include <stdio.h>

void ds1337_init( )
{
	char byte = 0;

	i2c_start();

	i2c_write(DS1337_I2C_ADDRESS);
	i2c_write(DS1337_REG_CONTROL);
	
	i2c_stop();
}

unsigned char ds1337_read_register(unsigned char reg)
{
	unsigned char data;

	i2c_start();
	i2c_write(DS1337_I2C_ADDRESS);
	i2c_write(reg);

	i2c_restart();
	i2c_write(DS1337_I2C_ADDRESS | 1);
	data = i2c_read(I2C_NAK);
	i2c_stop();

	return data;
}

 void ds1337_write_register(unsigned char reg, unsigned char data)
{
	i2c_start();
	i2c_write(DS1337_I2C_ADDRESS);
	i2c_write(reg);
	i2c_write(data);
	i2c_stop();
}

unsigned char ds1337_bcd_to_dec(unsigned char bcd, unsigned char ten_bits)
{
	unsigned char dec;

	dec = ((bcd >> 4) & 0xFF >> ten_bits) * 10;
	dec += (bcd & 0xFF >> 4);

	return dec;
}

unsigned char ds1337_dec_to_bcd(unsigned char dec, unsigned char ten_bits)
{
	unsigned char bcd;
	
	bcd = ((dec / 10) & 0xFF >> 8 - ten_bits) << 4;
	bcd += dec % 10;

	return bcd;	
}

unsigned char ds1337_get_hour()
{
	unsigned char reg, hour;

	reg = ds1337_read_register(DS1337_REG_HOURS);
	
	// Bit 6 of the hour register is 12/24 hour
	if (reg & 0x40) {
		// 12 hour, 1 bit for *10
		hour = ds1337_bcd_to_dec(reg, 1);
	} else {
		// 24 hour, 2 bits for *10
		hour = ds1337_bcd_to_dec(reg, 2);
	}

	return hour;
}

void ds1337_set_minutes(unsigned char minutes)
{
	minutes = ds1337_dec_to_bcd(minutes, 3);
	ds1337_write_register(DS1337_REG_MINUTES, minutes);
}

void ds1337_set_hour(unsigned char hour)
{
	//hour = 
	ds1337_write_register(DS1337_REG_MINUTES, hour);
}

unsigned char ds1337_get_day()
{
	unsigned char day;
	day = ds1337_read_register(DS1337_REG_DAY);
	return day;
}

unsigned char ds1337_set_alarm1(unsigned char hour, unsigned char minutes, unsigned char seconds, unsigned char day)
{
	return 1;
}

void ds1337_enable_oscillator()
{
	// Enable oscillator is 8th bit of control register, active low.
	unsigned char data = ds1337_read_register(DS1337_REG_CONTROL);
	data &= 0x7F;
	ds1337_write_register(DS1337_REG_CONTROL, data);

	// Also clear the status flag, without affecting the alarm
	// flags.
	unsigned char data = ds1337_read_register(DS1337_REG_STATUS);
	data &= 0x7F;
	ds1337_write_register(DS1337_REG_STATUS, 1);
}

void ds1337_disable_oscillator()
{
	// Inactive high, clear 8th bit of control register.
	unsigned char data = ds1337_read_register(DS1337_REG_CONTROL);
	data |= 0x80;
	ds1337_write_register(DS1337_REG_CONTROL, data);
}

void ds1337_clear_alarm1_flag()
{
	unsigned char status = ds1337_read_register(DS1337_REG_STATUS);
	status &= 0xFE;
	ds1337_write_register(DS1337_REG_STATUS, status);
}

void ds1337_clear_alarm2_flag()
{
	unsigned char status = ds1337_read_register(DS1337_REG_STATUS);
	status &= 0xFD;
	ds1337_write_register(DS1337_REG_STATUS, status);
}

void ds1337_enable_interrupts()
{
	unsigned char data = ds1337_read_register(DS1337_REG_CONTROL);
	//data |= 0x4;
// DEBUG: enable interrupts for A2
	data |= 0x6;
	ds1337_write_register(DS1337_REG_CONTROL, data);
}