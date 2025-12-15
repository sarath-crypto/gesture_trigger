#define F_CPU 8000000UL
/*
PB5 - 1 Reset
PB3 - 2 SCLK
PB4 - 3 DATA
GND - 4
PB0 - 5 SDA
PB1 - 6 LED/UART
PB2 - 7 SCL
VCC - 8
*/
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> 
#include <avr/eeprom.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "glb.h"

#define COM
#define GYRO

#ifdef 	GYRO
	#include "alg.h"
	#include "mpu6500.h"
#endif

#ifdef	COM
	#include "enc.h"
	#define DATA_PIN 		PB4
	#define SCLK_PIN		PB3
	#define FRAME_SZ		32
	#define BIT_DELAY 		40
	#define N 			2 
	#define T 			2 
	#define FCR 			0
	#define LED_COUNT		3
#endif

#ifdef DEBUG
	#define LED_PIN			PB5
#else
	#define LED_PIN			PB1
#endif

void (*reset)(void) = 0;
unsigned char ledc = 0;

ISR (TIMER0_OVF_vect){
	if (ledc)ledc--;
	else CLR(PORTB,LED_PIN);
}

#ifdef 	COM
void send_data(unsigned char cmd) {
	ledc = 250;
	SET(PORTB,LED_PIN);
	uint8_t data[RS_BLOCK_SIZE];
	data[0] = 0xf0 | cmd;
	enc com(T, FCR);
	com.RsEncode(data, N);
	data[1] = data[3];
	
	uint32_t bits = 0;
	for(int i = 0;i < 4;i++){
		bits = bits << 8;
		bits |= data[i];
	}
	for (int i = 0; i < FRAME_SZ; i++) {
		if (bits & 0x01)SET(PORTB,DATA_PIN);
		else CLR(PORTB,DATA_PIN);
		bits = bits >> 1;
        	SET(PORTB,SCLK_PIN);
		_delay_ms(BIT_DELAY);
        	CLR(PORTB,SCLK_PIN);
		_delay_ms(BIT_DELAY);
	}
}
#endif

int main(void){
#ifdef COM
        DDRB |= (1 << LED_PIN) | (1 << DATA_PIN) | (1 << SCLK_PIN);
#endif

#ifdef GYRO
        mpu6500 imu;
        if(!imu.state){
		SET(PORTB,LED_PIN);
		_delay_ms(1000);
		reset();
	}
	int8_t ay,az;
#endif 
	for(int i = 0;i < LED_COUNT;i++){
		FLP(PORTB,LED_PIN);
		_delay_ms(1000);
	}

	TCCR0A=0x00;   
	TCCR0B=0x00;
	TCCR0B |= (1<<CS00)|(1<<CS02);   
	sei(); 
	TCNT0=0;
	TIMSK|=(1<<TOIE0); 

        while(1){
#ifdef GYRO
		imu.getMotion();
		if(imu.sys.calc(&ay,&az)){
		       	if((ay == -16) && (az >= 0)){
#ifdef COM
				send_data(0);
#endif
			}
		}
#endif		
        }
        return 0;
}
