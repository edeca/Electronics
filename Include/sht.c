/* See sht.h for documentation */

#include <htc.h>
#include "sht.h"
#include <delay.h>

#include <stdio.h> // DEBUGGING
// 100ns delay for SCK high/low

void
_sht_InitiateBus() {
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
_sht_InterfaceReset() {
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
sht_SoftReset() {
    return sht_Command(SHT_SOFT_RESET);
}

unsigned char
_sht_ReadByte(char more) {
    unsigned int d = 0;

    // TRIS should be set to input when we get here, do we assume it or set it?
    SHT_DAT_TRIS = 1;

    // Read 8 bits, MSB first
    signed char i;
    for (i = 7; i >= 0; i--) {
        SHT_CLK_HIGH();
        if (SHT_DAT) d |= (1 << i);
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
sht_Command(char cmd) {
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
_sht_UpdateCRC(unsigned char crc, char data) {
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

sht_status_t
sht_ReadStatus() {
    sht_status_t status;
    status.value = 0;

    if (!sht_Command(SHT_READ_STATUS)) return status;

    //  printf("didn't return, continuing to read status byte\r\n");

    while (SHT_DAT);

    //  printf("finished waiting for dat to come low\r\n");

    status.value = _sht_ReadByte(SHT_LAST);

    return status;
}

unsigned short
sht_ReadTemperature() {
    unsigned short temperature;
    unsigned char data;
    unsigned char sht_crc = 0;

    if (!sht_Command(SHT_MEASURE_TEMPERATURE)) return 0;

    // Wait for DAT to become low, signalling end of conversion
    /** @todo Add a timeout here, max wait should be 80ms for humidity */
    //unsigned short timeout = 0xFFFF;
    while (SHT_DAT);

    // Initialise CRC with the command byte
    sht_crc = _sht_UpdateCRC(sht_crc, SHT_MEASURE_TEMPERATURE);

    // Sensor returns MSB, LSB then CRC
    data = _sht_ReadByte(SHT_MORE);
    sht_crc = _sht_UpdateCRC(sht_crc, data);
    temperature = data << 8;

    data = _sht_ReadByte(SHT_MORE);
    sht_crc = _sht_UpdateCRC(sht_crc, data);
    temperature |= data;

    // Sensor returns CRC reversed
    data = _sht_ReadByte(SHT_LAST);
    data = _sht_ReverseByte(data);

    // Check calculated vs. recieved CRC
    if (sht_crc != data) return 0;

    return temperature;
}

unsigned short
sht_ReadHumidity() {
    unsigned short humidity;
    unsigned char data;
    unsigned char sht_crc = 0;

    if (!sht_Command(SHT_MEASURE_HUMIDITY)) return 0;

    // Wait for DAT to become low, signalling end of conversion
    /** @todo Add a timeout here, max wait should be 80ms for humidity */
    //unsigned short timeout = 0xFFFF;
    while (SHT_DAT);

    // Initialise CRC with the command byte
    sht_crc = _sht_UpdateCRC(sht_crc, SHT_MEASURE_HUMIDITY);

    // Sensor returns MSB, LSB then CRC
    data = _sht_ReadByte(SHT_MORE);
    sht_crc = _sht_UpdateCRC(sht_crc, data);
    humidity = data << 8;

    data = _sht_ReadByte(SHT_MORE);
    sht_crc = _sht_UpdateCRC(sht_crc, data);
    humidity |= data;

    // Sensor returns CRC reversed
    data = _sht_ReadByte(SHT_LAST);
    data = _sht_ReverseByte(data);

    // Check calculated vs. recieved CRC
    if (sht_crc != data) return 0;

    return humidity;
}

void
sht_WriteStatus(sht_status_t status) {
    /* @todo Work out the write sequence */
}

float
sht_RelativeHumidity(short raw) {
    /* @todo These constants change for different Vdd and also for
     * 12 bit or 8 bit raw values. */
    float rh = -2.0468 + 0.0367 * raw + -1.5955E-6 * (raw * raw);
    return rh;
}

float
sht_CompensateHumidity(short raw, float rh, float temp) {
    /* 12bit raw humidity assumed */
    return (temp - 25)*(0.01 + 0.00008 * raw) + rh;
}

float
sht_TemperatureInCelcius(short raw) {
    /* @todo These constants change for different Vdd, 3V assumed here */
    return -39.6 + 0.01 * raw;
}

unsigned char
_sht_ReverseByte(unsigned char orig) {
    // From the excellent http://graphics.stanford.edu/~seander/bithacks.html
    unsigned int rev = orig; // r will be reversed bits of v; first get LSB of v
    //int shift = sizeof(orig) * 8 - 1; // extra shift needed at end
    int shift = 7;

    for (orig >>= 1; orig; orig >>= 1) {
        rev <<= 1;
        rev |= orig & 1;
        shift--;
    }
    rev <<= shift; // shift when v's highest bits are zero
    return rev;
}