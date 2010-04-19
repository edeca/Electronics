/********
 * Author: David Cannings <david@edeca.net>
 * 
 * BoostC code to use 24LCxxx EEPROM chips.  Uses the BoostC I2C libraries
 * which should be included and configured in the main project.  I2C works 
 * in either software or hardware mode.
 *
 * 19th April 2010 (rev1): Initial version
 ********/

#ifndef _24L_EEPROM_H
#define _24L_EEPROM_H

// Check if the EEPROM is busy (still writing)
//
// Arguments:
//   control_byte - the I2C write message with the correct address
//
// Returns:
//   TRUE if the EEPROM is still writing, FALSE otherwise
char ee24_is_busy(char control_byte) 
{
	i2c_start();
	char ack = i2c_write(control_byte);
	i2c_stop();
	return ack;
}

// Sequential write to a specific address
// Uses the page write mechanism to write up to 64 bytes
//
// Arguments:
//   device_address - 3 byte address of the EEPROM (a2/a1/a0)
//     data_address - where in the EEPROM to write data
//             data - pointer to a char buffer
//            count - how many bytes to write (0<count<=64)
//
// Returns: 
//   Nothing
//
// Note: Blocks if the EEPROM is busy completing a previous operation
//       until the write has finished
void ee24_write_sequential(char device_address, short data_address, char *data, unsigned char count)
{
	// TODO: ACK is returned by i2c_write, we should probably block until it has finished
	//		 See section 7 of the manual

	// Control byte:
	// 1010     - preamble
	//     AAA  - address
	//        W - read/write (0)
	char control_byte = 0b10100000;
	
	// Mix device address with the control byte
	control_byte |= (device_address << 1) & 0b00001110;

	// Wait if the EEPROM is still writing data
	while(ee24_is_busy(control_byte));

	i2c_start();
	i2c_write(control_byte);
	i2c_write(data_address >> 8); 				// send data HIGH address
	i2c_write((char) data_address & 0x00ff); 	// send data LOW address
	
	while(count) {
		i2c_write(*data++);
		count--;
	}
	
	i2c_stop();
}

// Write a single byte to a specific address
void ee24_write_byte(char device_address, short data_address, char byte)
{
	ee24_write_sequential(device_address, data_address, &byte, 1);
}

// Sequential read from a specific address
// Can read the whole device in this mode, but reads past the highest address will roll over
//
// Arguments:
//   see ee24_write_sequential
//
// Returns:
//   Nothing, data is written directly to data buffer
// 
// Note: Does not append a trailing NULL byte, so remember this if reading strings
void ee24_read_sequential(char device_address, short data_address, char *data, unsigned short count)
{
	char control_byte = 0b10100000;
	control_byte |= (device_address << 1) & 0b00001110;

	i2c_start();
	i2c_write(control_byte);
	i2c_write(data_address >> 8); 				// send data HIGH address
	i2c_write((char) data_address & 0x00ff); 	// send data LOW address
	
	// Restart the bus and issue the read command
	i2c_restart();
	i2c_write(control_byte | 0b0000001);
	
	// Read in bytes from the EEPROM, acknowledge all except the
	// last byte
	while (--count) {
		*data++ = i2c_read(0);
	}
	*data++ = i2c_read(1);
	
	i2c_stop();
}

// Write a single byte from a specific address
void ee24_read_byte(char device_address, short data_address, char byte)
{
	ee24_read_sequential(device_address, data_address, &byte, 1);
}

#endif