
// All DS1337 have a fixed address, the last bit
// is be the R/W bit
#define DS1337_I2C_ADDRESS 0b11010000
//#define DS1337_I2C_ADDRESS 0b00010000
//#define DS1337_I2C_ADDRESS 0b11000000

#define DS1337_REG_SECONDS 			0x00
#define DS1337_REG_MINUTES 			0x01
#define DS1337_REG_HOURS 			0x02
#define DS1337_REG_DAY 				0x03
#define DS1337_REG_DATE 			0x04
#define DS1337_REG_MONTH_CENTURY 	0x05
#define DS1337_REG_YEAR 			0x06
#define DS1337_REG_ALM1_SECONDS 	0x07
#define DS1337_REG_ALM1_MINUTES 	0x08
#define DS1337_REG_ALM1_HOURS 		0x09
#define DS1337_REG_ALM1_DAY_DATE 	0x0A
#define DS1337_REG_ALM2_MINUTES 	0x0B
#define DS1337_REG_ALM2_HOURS 		0x0C
#define DS1337_REG_ALM2_DAY_DATE 	0x0D
#define DS1337_REG_CONTROL 			0x0E
#define DS1337_REG_STATUS  			0x0F

/*
typedef struct {
	char hours;
	char minutes;
	char seconds;
} time_t;
*/

void ds1337_init( );
unsigned char ds1337_read_register(unsigned char reg);
void ds1337_write_register(unsigned char reg, unsigned char data);
unsigned char ds1337_bcd_to_dec(unsigned char bcd, unsigned char ten_bits);
unsigned char ds1337_dec_to_bcd(unsigned char dec, unsigned char ten_bits);
unsigned char ds1337_get_day();
unsigned char ds1337_get_hour();
void ds1337_set_minutes(unsigned char minutes);
void ds1337_enable_oscillator();
void ds1337_disable_oscillator();
void ds1337_enable_interrupts();
void ds1337_clear_alarm2_flag();
void ds1337_clear_alarm1_flag();