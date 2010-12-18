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
 *   // Retrieve humidity and convert to RH%.  See warnings about
 *   // floating point!
 *   unsigned short humidity_raw = sht_ReadHumidity();
 *	 float humidity_rh = sht_RelativeHumidity(humidity_raw);
 *  
 *   // Retrieve temperature and convert to degrees C.
 *   unsigned short temp_raw = sht_ReadTemperature();
 *   float temp_c = sht_TemperatureInCelcius(temp_raw);
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
 * Send the Sensiron protocol initiation sequence.
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
/**
 * A generic byte reversing routine.  Used to verify the CRC from the SHT sensor.
 *
 * Adapted from http://graphics.stanford.edu/~seander/bithacks.html
 * 
 * @param orig The byte to reverse.
 * @return The original byte in the opposite order.
 */
unsigned char _sht_ReverseByte(unsigned char orig);
/**
 * Read a single byte from the sensor.
 * 
 * @param more Use SHT_MORE if there are subsequent bytes to read or SHT_LAST for the last.
 * @return The byte as read from the data line.
 */
unsigned char _sht_ReadByte(char more);
/**
 * Read the current humidity from the sensor.  See sht_RelativeHumidity() for 
 * converting this to relative humidity.
 *
 * @return The current humidity as a raw value or 0 for CRC failure.
 */
unsigned short sht_ReadHumidity();
/**
 * Read the current temperature from the sensor.  See sht_TemperatureInCelcius() for
 * conversion to celcius.
 *
 * @return The current temperature as a raw value or 0 for CRC failure.
 */
unsigned short sht_ReadTemperature();
/**
 * Send a command to the SHT device.
 *
 * @param cmd The command (see defined SHT_ values).
 * @return 1 for success (SHT sensor acknowledged this command), 0 for failure.
 */
unsigned char sht_Command(char cmd);
/**
 * Write to the internal status register of the SHT device.
 *
 * @param status A sht_status_t to write to the SHT device.
 */
void sht_WriteStatus(sht_status_t status);
/**
 * Obtain the internal status register from the SHT device.
 *
 * @return A sht_status_t from the SHT device.
 */
sht_status_t sht_ReadStatus();
/**
 * Convert a raw reading to a floating point relative humidity
 * (\%RH).
 *
 * @warning This will link in the floating point library, which can
 *          add a considerable code size overhead.
 *
 * @param raw	Raw reading from sht_ReadHumidity()
 * @return A floating point relative humidity.
 */
float sht_RelativeHumidity(short raw);
/**
 * Convert a raw reading to a floating point temperature in degrees
 * celcius.
 *
 * Implements the algorithm from the datasheet.
 *
 * @warning This will link in the floating point library, which can
 *          add a considerable code size overhead.
 *
 * @param raw	Raw reading from sht_ReadTemperature()
 * @return A floating point temperature value in degrees C.
 */
float sht_TemperatureInCelcius(short raw);

#endif