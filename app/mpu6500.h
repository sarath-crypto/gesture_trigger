
#ifndef MPU9250_h
#define MPU9250_h

#include <stdint.h>
#include "i2c.h"
#include "alg.h"

#define MPU6500_ADDR		0x68
// MPU9250 registers
#define ACCEL_OUT  		0x3B
#define GYRO_OUT  		0x43
#define TEMP_OUT  		0x41
#define EXT_SENS_DATA_00  	0x49

#define ACCEL_CONFIG  		0x1C
#define ACCEL_FS_SEL_2G  	0x00
#define ACCEL_FS_SEL_4G  	0x08
#define ACCEL_FS_SEL_8G  	0x10
#define ACCEL_FS_SEL_16G 	0x18

#define GYRO_CONFIG  		0x1B
#define GYRO_FS_SEL_250DPS  	0x00
#define GYRO_FS_SEL_500DPS  	0x08
#define GYRO_FS_SEL_1000DPS  	0x10
#define GYRO_FS_SEL_2000DPS  	0x18

#define ACCEL_CONFIG2  		0x1D
#define ACCEL_DLPF_184  	0x01
#define ACCEL_DLPF_92  		0x02
#define ACCEL_DLPF_41  		0x03
#define ACCEL_DLPF_20  		0x04
#define ACCEL_DLPF_10  		0x05
#define ACCEL_DLPF_5  		0x06

#define CONFIG  		0x1A
#define GYRO_DLPF_184  		0x01
#define GYRO_DLPF_92  		0x02
#define GYRO_DLPF_41  		0x03
#define GYRO_DLPF_20  		0x04
#define GYRO_DLPF_10  		0x05
#define GYRO_DLPF_5  		0x06

#define SMPDIV  		0x19

#define INT_PIN_CFG  		0x37
#define INT_ENABLE  		0x38
#define INT_PULSE_50US  	0x00
#define INT_RAW_RDY_EN  	0x01

#define PWR_MGMNT_1  		0x6B
#define PWR_RESET  		0x80
#define CLOCK_SEL_PLL 	 	0x01

#define PWR_MGMNT_2  		0x6C
#define SEN_ENABLE  		0x00

#define USER_CTRL  		0x6A
#define I2C_MST_EN  		0x20
#define I2C_MST_CLK  		0x0D
#define I2C_MST_CTRL  		0x24
#define I2C_SLV0_ADDR  		0x25
#define I2C_SLV0_REG  		0x26
#define I2C_SLV0_DO  		0x63
#define I2C_SLV0_CTRL  		0x27
#define I2C_SLV0_EN  		0x80
#define I2C_READ_FLAG  		0x80

#define WHO_AM_I  		0x75

#define _tempScale 		333.87f
#define _tempOffset 		21.0f
#define _i2cRate 		400000

#define G 			9.807f
#define _d2r 			(3.14159265359f/180.0f)
#define	tX_0			0
#define tX_1			1
#define	tX_2			0
#define tY_0			1
#define tY_1			0
#define tY_2			0
#define tZ_0			0
#define tZ_1			0
#define	tZ_2			-1

enum mpu9250_config{
    GYRO_RANGE_250DPS = 1,
    GYRO_RANGE_500DPS,
    GYRO_RANGE_1000DPS,
    GYRO_RANGE_2000DPS,
    ACCEL_RANGE_2G,
    ACCEL_RANGE_4G,
    ACCEL_RANGE_8G,
    ACCEL_RANGE_16G,
    DLPF_BANDWIDTH_184HZ,
    DLPF_BANDWIDTH_92HZ,
    DLPF_BANDWIDTH_41HZ,
    DLPF_BANDWIDTH_20HZ,
    DLPF_BANDWIDTH_10HZ,
    DLPF_BANDWIDTH_5HZ
};

class mpu6500{
    private:
        float _accelScale;
	i2c bus;
        uint8_t whoAmI(void);
	void getMotionCounts(int16_t*,int16_t *);
    public:
	alg sys;
	bool writeRegister(uint8_t, uint8_t );
        bool readRegister(uint8_t,  uint8_t*);
	bool state;
        mpu6500();
        void getMotion(void);
};

#endif
