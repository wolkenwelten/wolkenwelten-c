#pragma once
#include "../stdint.h"

#pragma pack(push, 1)
typedef struct {
	u32 typesize;

	/*
	 *  type     = typesize & 0xFF
	 *  size     = typesize >> 10
	 */

	union {
		float f[1030];
		i32   i[1030];
		u32   u[1030];
		u16   s[2060];
		u8    c[4120];
	} val;
} packet;

#pragma pack(pop)
inline uint alignedLen(uint size){
	if(size & 0x3){
		return (size & (~0x3))+4;
	}else{
		return (size & (~0x3));
	}
}
inline uint packetLen(const packet *p){
	return p->typesize >> 10;
}
inline uint packetType(const packet *p){
	return p->typesize & 0xFF;
}
inline void packetSet(packet *p, u8 ptype, u32 len){
	p->typesize = (u32)ptype | (len << 10);
}

void packetQueue         (packet *p, u8 ptype, u32 len, int c);
void packetQueueExcept   (packet *p, u8 ptype, u32 len, int c);
void packetQueueToServer (packet *p, u8 ptype, u32 len);
