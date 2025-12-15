#ifndef I2C_H
#define I2C_H

#define F_CPU	8000000UL

#include <avr/io.h>
#include <stdbool.h>

#define I2C_ACK 0x00
#define I2C_NACK 0xFF

// These numbers are from a calculation chart
// from I2C spec -- NXP I2C Timing Specification
#ifdef I2C_FAST_MODE // 400kHz fast
  // low period of SCL
  #define T2_TWI 2 // >1.3microseconds
  // high period of SCL
  #define T4_TWI 1 // >0.6microseconds
#else // 100kHz standard
  // low period of SCL
  #define T2_TWI 5 // >4.7microseconds
  // high period of SCL
  #define T4_TWI 4 // >4.0microseconds
#endif

#ifndef i2c_ddr
  #define i2c_ddr DDRB
#endif
#ifndef i2c_port
  #define i2c_port PORTB
#endif
#ifndef i2c_pin
  #define i2c_pin PINB
#endif
#ifndef i2c_scl
  #define i2c_scl PORTB2
#endif
#ifndef i2c_sda
  #define i2c_sda PORTB0
#endif
#ifndef i2c_data
  #define i2c_data USIDR
#endif
#ifndef i2c_status
  #define i2c_status USISR
#endif
#ifndef i2c_control
  #define i2c_control USICR
#endif

class i2c{
private:
	unsigned char transfer(unsigned char);
public:
	i2c();
	bool start(void);
	void stop(void);
	unsigned char write_byte(unsigned char);
	unsigned char read_byte(bool);
	unsigned char write_address(unsigned char,bool);
};
#endif
