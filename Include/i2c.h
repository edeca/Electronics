#define I2C_ACK 0
#define I2C_NAK 1

void i2c_init();
void i2c_start();
void i2c_restart();
void i2c_stop();
char i2c_read(char ack);
char i2c_write(char data);
void i2c_resync();