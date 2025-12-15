#ifndef ENC_H_
#define ENC_H_

#include <stdint.h>
#include <stdbool.h>

#define RS_MAX_REDUNDANCY_BYTES 	2
#define RS_BLOCK_SIZE 			4
#define RS_MAX_DATA_SIZE (RS_BLOCK_SIZE - RS_MAX_REDUNDANCY_BYTES)

typedef struct LwFecRS{
    uint8_t generator[RS_MAX_REDUNDANCY_BYTES + 1]; 
    uint8_t T;
    uint8_t fcr; 
}LwFecRs;

class enc{
private:
	LwFecRs rs;
	uint8_t GfAdd(uint8_t,uint8_t);
	uint8_t GfMul(uint8_t,uint8_t);
	uint8_t GfPow2(uint8_t);
	uint8_t *GfPolyDiv(uint8_t *,uint8_t,uint8_t *,uint8_t,uint8_t *);
	void    GfPolyMul(uint8_t *,uint8_t,uint8_t *,uint8_t,uint8_t *);
public:
	void RsEncode(uint8_t *,uint8_t);
	enc(uint8_t, uint8_t);
};

#endif
