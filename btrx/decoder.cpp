#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <iostream>
#include "decoder.h"


uint8_t decoder::GfAdd(uint8_t x, uint8_t y){
	return x ^ y; 
}
uint8_t decoder::GfSub(uint8_t x, uint8_t y){
	return x ^ y; 
}
uint8_t decoder::GfMul(uint8_t x, uint8_t y){
	if((x == 0) || (y == 0))return 0;
	return GfExp[(GfLog[x] + GfLog[y]) % 255];
}
uint8_t decoder::GfDiv(uint8_t dividend, uint8_t divisor){
	if(divisor == 0)return 0; 
	if(dividend == 0)return 0;
	return GfExp[(255 + GfLog[dividend] - GfLog[divisor]) % 255];
}
uint8_t decoder::GfPow(uint8_t x, uint8_t exponent){
	return GfExp[(exponent * GfLog[x]) % 255];
}
uint8_t decoder::GfPow2(uint8_t exponent){
	return GfExp[exponent];
}
uint8_t decoder::GfInv(uint8_t x){
	return GfExp[255 - GfLog[x]];
}

void decoder::GfPolyScale(uint8_t *p, uint8_t o, uint8_t s, uint8_t *out){
	for(uint8_t i = 0; i < o; i++)out[i] = GfMul(p[i], s);
}

uint8_t decoder::GfPolyAdd(uint8_t *p1, uint8_t o1, uint8_t *p2, uint8_t o2, uint8_t *out){
	if(o1 >= o2){
		for(uint8_t i = 0; i < o2; i++)out[i] = p1[i] ^ p2[i];
		return o1;
	}else{
		for(uint8_t i = 0; i < o1; i++)out[i] = p1[i] ^ p2[2];
		return o2;
	}
}

void decoder::GfPolyMul(uint8_t *p1, uint8_t o1, uint8_t *p2, uint8_t o2, uint8_t *out){
	memset(out, 0, o1 + o2 - 1);
	for(uint8_t i = 0; i < o1; i++){
		for(uint8_t j = 0; j < o2; j++)out[i + j] ^= GfMul(p1[i], p2[j]); 
	}
}

uint8_t decoder::GfPolyEval(uint8_t *p, uint8_t o, uint8_t x){
	uint8_t ret = p[0];
	for(uint8_t i = 1; i < o; i++)ret = GfAdd(GfMul(ret, x), p[i]);
	return ret;
}

void decoder::GfPolyInv(uint8_t *p, uint8_t o){
	for(uint8_t i = 0; i < (o >> 1); i++){
		uint8_t tmp = p[i];
		p[i] = p[o - i - 1];
		p[o - i - 1] = tmp;
	}
}

void decoder::syndromes(uint8_t *data, uint8_t size, uint8_t *out){
	for(uint8_t i = 0; i < rs.T; i++)out[i] = GfPolyEval(data, size, GfPow2(i + rs.fcr));
}

bool decoder::errorEvaluator(uint8_t *locator, uint8_t locatorSize, uint8_t *out){
	uint8_t pos = 0;
	for(uint8_t i = 0; i < RS_BLOCK_SIZE; i++){
		uint8_t lambda = 0;
		for(uint8_t j = 0; j < locatorSize; j++)lambda ^= GfPow2((GfLog[locator[locatorSize - j - 1]] + i * j) % 255);
		if(lambda == 0)out[pos++] = RS_BLOCK_SIZE - i - 1;
	}
	if(pos != (locatorSize - 1))return false;
	return true;
}


bool decoder::errorLocator(uint8_t *syndromes, uint8_t *out, uint8_t *outSize){
    uint8_t *errLoc = commonBuffer;
    uint8_t *newLoc = errLoc + RS_MAX_REDUNDANCY_BYTES + 1;
    uint8_t *oldLoc = newLoc + RS_MAX_REDUNDANCY_BYTES + 1;
    uint8_t *tmpLoc = oldLoc + RS_MAX_REDUNDANCY_BYTES + 1;

    memset(errLoc, 0, rs.T + 1);
    memset(newLoc, 0, rs.T + 1);
    memset(oldLoc, 0, rs.T + 1);
    memset(tmpLoc, 0, rs.T + 1);

    uint8_t newLocLen = 0;
    uint8_t errLocLen = 1;
    uint8_t oldLocLen = 1;

    errLoc[0] = 1;
    oldLoc[0] = 1;

    for(uint8_t i = 0; i < rs.T; i++){
        uint8_t delta = syndromes[i];
        for(uint8_t j = 1; j < errLocLen; j++)delta = GfSub(delta, GfMul(errLoc[j], syndromes[i - j]));

        for(uint8_t j = 0; j < oldLocLen; j++)oldLoc[oldLocLen - j] = oldLoc[oldLocLen - j - 1];
        oldLoc[0] = 0;

        oldLocLen++;

        if(delta != 0){
            if(oldLocLen > errLocLen){
                GfPolyScale(oldLoc, oldLocLen, delta, newLoc);
                newLocLen = oldLocLen;
                GfPolyScale(errLoc, errLocLen, GfInv(delta), oldLoc);
                oldLocLen = errLocLen;
                memcpy(errLoc, newLoc, newLocLen);
                errLocLen = newLocLen;
            }
            GfPolyScale(oldLoc, oldLocLen, delta, newLoc);
            newLocLen = oldLocLen;
            memcpy(tmpLoc, errLoc, errLocLen);
            errLocLen = GfPolyAdd(tmpLoc, errLocLen, newLoc, newLocLen, errLoc);
        }
    }

    uint8_t index = 0;
    for(uint8_t i = 0; i < errLocLen; i++){
        if((index == 0) && (errLoc[i] == 0))continue;
        out[index++] = errLoc[i];
    }
    if(((errLocLen - 1) << 1) > rs.T)return false;
    *outSize = errLocLen;
    return true;
}


bool decoder::fix(uint8_t *data, uint8_t size, uint8_t *syn, uint8_t *evaluator, uint8_t errCount){
	uint8_t *locator = commonBuffer;
	uint8_t *errataEvaluator = locator + RS_MAX_REDUNDANCY_BYTES + 1;
	memset(locator, 0, rs.T + 1);
	memset(errataEvaluator, 0, 2 * rs.T + 2);
	locator[0] = 1; 
	uint8_t locatorSize = 1;

	for(uint8_t i = 0; i < errCount; i++){
		memcpy(errataEvaluator, locator, rs.T);
		uint8_t t2[] = {GfPow(2, size - 1 - evaluator[i]), 1};
		GfPolyMul(errataEvaluator, locatorSize, t2, 2, locator);
		locatorSize++;
	}
	memset(errataEvaluator, 0, 2 * rs.T + 2);


	GfPolyInv(syn, rs.T);
	GfPolyMul(syn, rs.T, locator, locatorSize, errataEvaluator);
	for(uint8_t i = 0; i < locatorSize; i++)errataEvaluator[i] = errataEvaluator[rs.T + i];


	uint8_t errataPosition[RS_MAX_REDUNDANCY_BYTES];
	for(uint8_t i = 0; i < errCount; i++)errataPosition[i] = GfPow(2, size - 1 - evaluator[i]);

	uint8_t *errLocPrimePoly = syn; 
	uint8_t errLocPrimePolyLen = 0;

	for(uint8_t i = 0; i < errCount; i++){
		errLocPrimePolyLen = 0;
		uint8_t errataInv = GfInv(errataPosition[i]);

		for(uint8_t j = 0; j < errCount; j++)if(j != i)errLocPrimePoly[errLocPrimePolyLen++] = GfSub(1, GfMul(errataInv, errataPosition[j]));
		uint8_t errLocPrime = 1;
		for(uint8_t j = 0; j < errLocPrimePolyLen; j++)errLocPrime = GfMul(errLocPrime, errLocPrimePoly[j]);

		uint8_t y = GfPolyEval(errataEvaluator, locatorSize, errataInv);
		if(rs.fcr == 0)y = GfMul(y, errataPosition[i]);
		else if(rs.fcr > 0)y = GfMul(y, GfPow(errataInv, rs.fcr - 1));
		if(errLocPrime == 0)return false;
		data[evaluator[i]] = GfSub(data[evaluator[i]], GfDiv(y, errLocPrime));
	}
	return true;
}

bool decoder::checkSyndromes(uint8_t *syndromes, uint8_t size){
	bool err = false;
	for(uint8_t i = 0; i < size; i++){
		if(syndromes[i] != 0)err = true;
	}
	return !err;
}

bool decoder::RsDecode(uint8_t *data, uint8_t size, uint8_t *fixed){
	if((size > (RS_BLOCK_SIZE - rs.T)) || (rs.T > RS_MAX_REDUNDANCY_BYTES))return false;

	static uint8_t syn[RS_MAX_REDUNDANCY_BYTES + 1];
	static uint8_t locator[RS_MAX_REDUNDANCY_BYTES + 1];
	static uint8_t evaluator[RS_MAX_REDUNDANCY_BYTES + 1];

	memset(syn, 0, rs.T + 1);
	memset(locator, 0, rs.T + 1);
	memset(evaluator, 0, rs.T + 1);

	memmove(&data[RS_BLOCK_SIZE - rs.T], &data[size], rs.T);
	memset(&data[size], 0, RS_BLOCK_SIZE - size - rs.T);

	syndromes(data, RS_BLOCK_SIZE, syn); 
	if(checkSyndromes(syn, rs.T))return true;
	uint8_t locatorSize = 0;
	if(!errorLocator(syn, locator, &locatorSize)) return false;
	if(!errorEvaluator(locator, locatorSize, evaluator)) return false;
	if(!fix(data, RS_BLOCK_SIZE, syn, evaluator, locatorSize - 1))return false;
	syndromes(data, RS_BLOCK_SIZE, syn);
	if(checkSyndromes(syn, rs.T)){
		*fixed = locatorSize - 1;
		return true;
	}else return false;
}

decoder:: decoder(uint8_t T, uint8_t fcr){
	if(T > RS_MAX_REDUNDANCY_BYTES)return;
	static uint8_t temp[RS_MAX_REDUNDANCY_BYTES + 1];
	memset(rs.generator, 0, T + 1);
	rs.generator[0] = 1;
	for(uint8_t i = 0; i < T; i++){
		memcpy(temp, rs.generator, i + 1);
		uint8_t t2[]={1, GfPow2(i + fcr)};
		GfPolyMul(temp, i + 1,t2, 2, rs.generator);
	}
	rs.T = T;
	rs.fcr = fcr;
}


#if RS_BLOCK_SIZE > 256
#error Architectural limit of RS FEC block size is 256 bytes
#endif

#if RS_MAX_DATA_SIZE <= 0
#error RS FEC parity byte count must be less than the block size
#endif

