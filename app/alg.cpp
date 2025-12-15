#include <stdio.h>
#include <stdlib.h>
#include "alg.h"

alg::alg(){
	pw = 0;
	pr = 0;
	ps = 0;
}

void alg::ring_wr(data *pdata){
	ring[pw].ax = pdata->ax;
	ring[pw].ay = pdata->ay;
	ring[pw].az = pdata->az;
	pw++;
	pw = pw%16;
	if(ps < 15){
		ps++;
		full = false;
	}else if(pw == 1){
		full = true;
		pr++;
		ps++;
	}
}

bool alg::ring_rd(data *pdata){
	if(ps){
		pdata->ax = ring[pr].ax;		
		pdata->ay = ring[pr].ay;		
		pdata->az = ring[pr].az;		
		if(pr < 15)pr++;
		else pr = 0;
		ps--;
		return true;
	}
	return false;
}

bool alg::calc(int8_t *ay,int8_t *az){
	data sens;
	if(!full)return false;
	unsigned long int sy = 0;
	unsigned long int sz = 0;
	for(int i = 0;i < DATA_SZ;){
		if(ring_rd(&sens)){
			sy += sens.ay;
			sz += sens.az;
			i++;
		}
	}
	*ay = (int8_t)((float)sy/(float)DATA_SZ);
	*az = (int8_t)((float)sz/(float)DATA_SZ);
	return true;
}
