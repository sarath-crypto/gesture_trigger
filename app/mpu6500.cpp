#define F_CPU	8000000UL
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "mpu6500.h"

/* mpu6500 object, input the I2C address and I2C bus */
mpu6500::mpu6500(){
	state = false;
	writeRegister(PWR_MGMNT_1,PWR_RESET);
	_delay_ms(200);
	if(!writeRegister(PWR_MGMNT_1,CLOCK_SEL_PLL))return;
	if(whoAmI() != 112 )return;
	if(!writeRegister(PWR_MGMNT_2,SEN_ENABLE))return;
	if( !writeRegister(ACCEL_CONFIG,ACCEL_FS_SEL_8G) )return;
	_accelScale = G * 8.0f/32767.5f; // setting the accel scale to 8G
	// enable I2C master mode
	if( !writeRegister(USER_CTRL,I2C_MST_EN) )return;
	// set the I2C bus speed to 400 kHz
	if( !writeRegister(I2C_MST_CTRL,I2C_MST_CLK) )return;

	if( !writeRegister(ACCEL_CONFIG2,ACCEL_DLPF_5) )return;
	if( !writeRegister(CONFIG,GYRO_DLPF_5) )return;
	/* setting the sample rate divider */
	if( !writeRegister(SMPDIV,1))return;
	/* setting the interrupt */
	if( !writeRegister(INT_PIN_CFG,INT_PULSE_50US) )return;
	if( !writeRegister(INT_ENABLE,INT_RAW_RDY_EN) )return;

	state = true;
}


/* get accelerometer and gyro data given pointers to store values, return data as counts */
void mpu6500::getMotionCounts(int16_t* ay, int16_t* az){
	uint8_t buff[6];
	int16_t axx,ayy, azz;
	for(unsigned int  i = 0;i < sizeof(buff);i++)readRegister(ACCEL_OUT+i,&buff[i]); 
	axx = (((int16_t)buff[0]) << 8) | buff[1];  // combine into 16 bit values
	ayy = (((int16_t)buff[2]) << 8) | buff[3];
	azz = (((int16_t)buff[4]) << 8) | buff[5];
	*ay = tY_0*axx + tY_1*ayy + tY_2*azz;
	*az = tZ_0*axx + tZ_1*ayy + tZ_2*azz;
}
/* get accelerometer and gyro data given pointers to store values */
void mpu6500::getMotion(void){
	int16_t accel[2];
	getMotionCounts(&accel[0], &accel[1]);
	data sens;
	sens.ay = (uint8_t)(((float) accel[0]) * _accelScale);
	sens.az = (uint8_t)(((float) accel[1]) * _accelScale);
	sys.ring_wr(&sens);
}

/* writes a byte to mpu6500 register given a register address and data */
bool mpu6500::writeRegister(uint8_t subAddress, uint8_t data){
        if (!bus.start())return false;
        bus.write_address(MPU6500_ADDR, true);
        bus.write_byte(subAddress);
        bus.write_byte(data);
	bus.stop();
	return true;
}

/* reads registers from mpu6500 given a starting register address, number of bytes, and a pointer to store data */
bool mpu6500::readRegister(uint8_t subAddress,uint8_t *dest){
	if (!bus.start())return false;
	bus.write_address(MPU6500_ADDR, true);
	bus.write_byte(subAddress);
	if (!bus.start())return false;
	bus.write_address(MPU6500_ADDR, false);
	*dest = bus.read_byte(false);
	bus.stop();
	return true;
}

/* gets the mpu6500 WHO_AM_I register value, expected to be 0x71 */
uint8_t mpu6500::whoAmI(){
	uint8_t data;
	// read the WHO AM I register
	readRegister(WHO_AM_I,&data);
	// return the register value
	return data;
}
