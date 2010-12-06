/**
 * @file   sht.h
 * @author David <david@edeca.net>
 * @date   November, 2010
 * @brief  Header for Sensiron SHT temperature/humidity sensor library.
 * @sa     <a href="http://www.sensirion.com/en/pdf/product_information/Datasheet-humidity-sensor-SHT7x.pdf">SHT71 datasheet</a>
 * @details
 *
 * A library for accessing the SHT71 temperature sensor, 
 * using Sensiron's custom bus protocol (similar to i2c).
 *
 * The SHT devices are not addressable.  If you want multiple 
 * devices on the same bus, consider using a switched Vdd 
 * (perhaps with a small FET) and share the data and clock lines.
 * If you adopt this approach, allow XXms for sensor startup.
 * 
 * Example Usage:
 * @code
 *   // TODO
 * @endcode
 *
 * @todo Check all timings, 1us is used at present
 */

#ifndef _SHT_H
#define _SHT_H

/* 
 * User definable settings
 */

#define SHT_CLK LATB1
#define SHT_DAT RB0
#define SHT_DAT_TRIS TRISB0

/*
 * Macros for the Sensiron bus protocol
 */

#define SHT_DAT_HIGH() SHT_DAT_TRIS = 1; SHT_DAT = 0
#define SHT_DAT_LOW() SHT_DAT_TRIS = 0; SHT_DAT = 0
#define SHT_CLK_HIGH() SHT_CLK = 1
#define SHT_CLK_LOW() SHT_CLK = 0

/*
 * Constants, do not edit below this line
 */

/** Convert temperature command. */
#define SHT_MEASURE_TEMPERATURE 0x03
/** Convert humidity command. */
#define SHT_MEASURE_HUMIDITY 0x05
/** Write status register command. */
#define SHT_WRITE_STATUS 0x06
/** Read status register command. */
#define SHT_READ_STATUS 0x07
/** Soft reset the interface, clearing the status register.  Wait 11ms before sending another command. */
#define SHT_SOFT_RESET 0x1E

/** Used with ReadByte() if we plan on fetching more data after this byte. */
#define SHT_MORE 1
/** Used with ReadByte() if this is the last byte we will read. */
#define SHT_LAST 0

/** The status register */
typedef union
{
  struct
  {
    /** Reserved */
    unsigned reserved:1;
    /** Low battery detection (1 for Vdd < 2.47v).  Read only. */
    unsigned low_battery:1;
    /** Reserved */
    unsigned reserved1:1;
    /** Reserved */
    unsigned reserved2:1;
    /** For testing (do not use) */
    unsigned testing:1;
    /** Heater enable */
    unsigned heater:1;
    /** No reload from OTP */
    unsigned reload:1;
    /** ADC resolution. 1 = 8bit RH, 12bit temp, 0 = 12bit RH, 14bit temp. */
    unsigned resolution:1;
  } bits;
  unsigned char value;
} sht_status_t;  

/**
 * 
 */
void _sht_InitiateBus();
/**
 * Reset communication with the SHT device.  Does not affect the internal status
 * register.
 */
void _sht_InterfaceReset();
/**
 * Reset the SHT device, including the status register.
 *
 * @return 1 for success (SHT sensor acknowledged this command), 0 for failure.
 */
unsigned char sht_SoftReset();
unsigned char _sht_ReadByte(char more);
unsigned short sht_ReadHumidity();
unsigned short sht_ReadTemperature();
unsigned char sht_Command(char cmd);
void sht_WriteStatus(sht_status_t status);
sht_status_t sht_ReadStatus();
/**
 * Convert a raw reading to a floating point relative humidity
 * (%RH).
 *
 * @warning This will link in the floating point library, which can
 *          add a considerable code size overhead.
 *
 * @param raw	Raw reading from sht_ReadHumidity()
 */
float sht_RelativeHumidity(short raw);
float sht_TemperatureInCelcius(short raw);
unsigned char _sht_ReverseByte(unsigned char orig);

#endif