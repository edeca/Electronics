/* See sht.h for documentation */

#include <htc.h>
#include "sht.h"
#include "delay.h"

#include <stdio.h> // DEBUGGING
// 100ns delay for SCK high/low

void
_sht_InitiateBus()
{
  /** @todo Check how i2c handles delays at faster clock speed */
  SHT_CLK_LOW();

  SHT_DAT_HIGH();
  SHT_CLK_HIGH();
  DelayUs(1);

  SHT_DAT_LOW();
  DelayUs(1);

  // Toggle clock while data is low
  SHT_CLK_LOW();
  DelayUs(1);
  SHT_CLK_HIGH();
  DelayUs(1);

  SHT_DAT_HIGH();
  DelayUs(1);

  SHT_CLK_LOW();
  DelayUs(1);
}

void
_sht_InterfaceReset()
{
  // Toggle CLOCK 9 times with DATA high
  SHT_DAT_HIGH();
  SHT_CLK_LOW();
  
  for (int i = 0; i < 9; i++) {
    SHT_CLK_HIGH();
    asm("nop");
    SHT_CLK_LOW();
    asm("nop");
  }
  
  // Then issue a standard bus initiation
  _sht_InitiateBus();
}

unsigned char
_sht_ReadByte(char more)
{
  unsigned int d = 0;
  
  // TRIS should be set to input when we get here, do we assume it or set it?
  SHT_DAT_TRIS = 1;
  
  for (int i = 0; i < 8; i++) {
    SHT_CLK_HIGH();
    if (SHT_DAT) d |= 0x80;
    d >>= 1;
    SHT_CLK_LOW();
  }
 
  // Acknowledge only if we want to continue reading data
  if (more) SHT_DAT_LOW();
  
  // Toggle clock for ACK bit
  SHT_CLK_HIGH();
  DelayUs(1);
  SHT_CLK_LOW();
  DelayUs(1);
  
  SHT_DAT_TRIS = 1;
  
  return d;
}

unsigned char
sht_SoftReset()
{
  return sht_Command(SHT_SOFT_RESET);
}

unsigned char
sht_Command(char cmd)
{
  int ret = 0;

  _sht_InitiateBus();

  // Possibly unnecessary (check)
  SHT_CLK_LOW();

  // Send 8 bit command, MSB first
  for (int i = 0; i < 8; i++) {
    if (cmd & 0x80) {
      SHT_DAT_HIGH();
    } else {
      SHT_DAT_LOW();
	}
	cmd <<= 1;
    
    SHT_CLK_HIGH();
    DelayUs(1);
    SHT_CLK_LOW();
  }

  // Set TRIS for DAT line to input
  SHT_DAT_TRIS = 1;
  
  // ACK clock pulse
  SHT_CLK_HIGH();
  DelayUs(1);

  // Sensor pulls DAT low to acknowledge the command, return 1 for success
  if (!SHT_DAT) ret = 1;

  SHT_CLK_LOW();

  return ret;
}

unsigned char
_sht_UpdateCRC(unsigned char crc, char data)
{
  const char crc_mask = 0x31;
  char bit_mask;
	 			
  for (bit_mask = 0x80; bit_mask; bit_mask >>= 1) {
    if ((crc ^ data) & 0x80) {
      crc <<= 1;
	  crc ^= crc_mask;
    } else {
      crc <<= 1;
    }
    data <<= 1;
  }
  return crc;
}

unsigned short
sht_ReadTemperature()
{
  unsigned short temperature;
  
  if (!sht_Command(SHT_MEASURE_TEMPERATURE)) return 0;
  
  // Wait for DAT to become low, signalling end of conversion
  /** @todo Add a timeout here, max wait should be 80ms for humidity */
  //unsigned short timeout = 0xFFFF;
  while(SHT_DAT);

  // Sensor returns MSB, LSB then CRC
  temperature = _sht_ReadByte(SHT_MORE) << 8;
  temperature |= _sht_ReadByte(SHT_MORE);
  unsigned char crc = _sht_ReadByte(SHT_LAST);

  /** @todo Check CRC */

  return temperature;
}

unsigned short
sht_ReadHumidity()
{
  unsigned short humidity;
  unsigned int tmp;  

  if (!sht_Command(SHT_MEASURE_HUMIDITY)) return 0;
  
  // Wait for DAT to become low, signalling end of conversion
  /** @todo Add a timeout here, max wait should be 80ms for humidity */
  //unsigned short timeout = 0xFFFF;
  while(SHT_DAT);

  sht_crc = _sht_UpdateCRC(sht_crc, SHT_MEASURE_HUMIDITY);

  // Sensor returns MSB, LSB then CRC
  tmp = _sht_ReadByte(SHT_MORE);
  sht_crc = _sht_UpdateCRC(sht_crc, tmp);
  humidity = tmp << 8;

printf("humidity MSB is %d\r\n", tmp);

  tmp = _sht_ReadByte(SHT_MORE);
  sht_crc = _sht_UpdateCRC(sht_crc, tmp);
  humidity |= tmp;

printf("humidity LSB is %d\r\n", tmp);

  tmp = _sht_ReadByte(SHT_LAST);

printf("sht_Crc is now %d and received CRC was %d\r\n", sht_crc, tmp);

  /** @todo Check CRC */

  return humidity;
}

void
sht_WriteStatus(sht_status status)
{


}

sht_status
sht_ReadStatus()
{
  sht_status s;

  return s;

}