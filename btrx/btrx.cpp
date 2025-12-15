#include <iostream>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <mutex>
#include <cstring>

#include "decoder.h"

#define MOUSE_PATH	"/dev/input/event2"

#define N		2
#define T               2
#define FCR             0
#define FRAME_SZ	32

using namespace std;
enum sync{INIT = 0,SYNC,SYNC1,SYNC2};
/*
uint32_t reverse_u32(uint32_t n){
        uint32_t result = 0;
        for (int i = 0; i < 32; i++) {
                result <<= 1;
                result |= (n & 1);
                n >>= 1;
        }
        return result;
}
*/

uint8_t reverse_u8(uint8_t b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

int main(void){
	decoder com(T, FCR);

       	struct input_event ev;
        fd_set readfds;

	struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
		
	char name[256] = "Unknown";
	int fd = open(MOUSE_PATH, O_RDONLY | O_NONBLOCK);
        if (fd == -1)return 1;
        ioctl(fd, EVIOCGNAME(sizeof(name)),name);
	vector <bool> rxbuf;
	bool ch = false;
	uint8_t sync = INIT;
	int c = 0;
	while(1){
		FD_ZERO(&readfds);
                FD_SET(fd,&readfds);
                int ev_size = sizeof(struct input_event);
                int ret = select(fd + 1, &readfds, NULL, NULL, &tv);
                if(ret == -1){
			close(fd);
			break;
		}else if(read(fd, &ev, ev_size) >= ev_size){
			if(ev.type == 1) {
				if(ev.code == 272)ch = !ev.value;
				if((ev.code == 273) && (ev.value == 1))rxbuf.push_back(ch);
			}
		}
		if(rxbuf.size() >= FRAME_SZ){
			for(int i = 0;i < rxbuf.size();i++){
				if(rxbuf[i])printf("1");
				else printf("0");
			}
			printf("\n",rxbuf.size());

			unsigned int data32;
			unsigned char data[4];
			unsigned char odata[4];
			for(int i = 0;i < FRAME_SZ;i++){
				data32 = data32 << 1;
				data32 |= rxbuf[i];
			}
			memcpy(data,(void *)&data32,sizeof(data32));
			data[0] = reverse_u8(data[0]); 
			data[1] = reverse_u8(data[1]); 
			data[2] = reverse_u8(data[2]); 
			data[3] = reverse_u8(data[3]); 
		
			memcpy(odata,data,4);
			printf("%02x %02x %02x %02x [%d %d]\n",odata[0],odata[1],odata[2],odata[3],rxbuf.size(),c);
			printf("%02x %02x %02x %02x\n",data[0],data[1],data[2],data[3]);
			c++;
			unsigned char bf = 0;
			if(com.RsDecode(data, N,&bf)){
				if((data[0] >> 4) ==  0x0f){
					rxbuf.erase(rxbuf.begin(),rxbuf.begin()+FRAME_SZ);
					sync = SYNC1;
				}
			}
			if(sync != SYNC1){
				data[3] = data[1];
				data[1] = 0;
				if(com.RsDecode(data, N,&bf)){
					if((data[0] >> 4) == 0x0f){
						rxbuf.erase(rxbuf.begin(),rxbuf.begin()+FRAME_SZ);
						sync = SYNC2;
					}
				}else sync = INIT;
			}
			if(!sync)rxbuf.erase(rxbuf.begin());
			else{
				printf("Odata   %02x %02x %02x %02x\n",odata[0],odata[1],odata[2],odata[3]);
				printf("Decoded %02x %02x %02x %02x [%d %d]\n",data[0],data[1],data[2],data[3],bf,sync);
				data[0] = data[0] << 4;
				data[0] = data[0] >> 4;
				printf("Data->%02x\n",data[0]);
				sync = SYNC;
				c = 0;
			}
		}	
	}
	return 0;
}

