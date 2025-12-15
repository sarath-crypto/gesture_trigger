#ifndef __GLB_H
#define __GLB_H


#define SET(reg, pos) (reg |= 1<<(pos))
#define FLP(reg, pos) (reg ^= 1<<(pos))
#define CLR(reg, pos) (reg &= ~(1<<(pos)))
#define GET(reg, pos) (reg &  1<<(pos))

#endif 
