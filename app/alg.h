#ifndef ALG_H
#define ALG_H

#define BUF_SZ		16
#define DATA_SZ		BUF_SZ
#define DATA_DCR 	((float)10.0/(float)DATA_SZ)


typedef struct data{
	int8_t ax;
	int8_t ay;
	int8_t az;
}data;

class alg{
	private:
        uint8_t pr;
        uint8_t pw;
        uint8_t ps;
	bool full;
  	data ring[BUF_SZ];
	bool ring_rd(data *);
	public:
	alg();
	void ring_wr(data *);
	bool calc(int8_t *,int8_t *);
};

#endif
