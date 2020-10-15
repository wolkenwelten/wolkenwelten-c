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
		i32   i32[1030];
		u32   u32[1030];
		u16   u16[2060];
		i16   i16[2060];
		u8    u8[4120];
		i8    i8[4120];
	} v;
} packet;

#pragma pack(pop)
static inline uint alignedLen(uint size){
	if(size & 0x3){
		return (size & (~0x3))+4;
	}
	return size;
}
static inline uint packetLen(const packet *p){
	return p->typesize >> 10;
}
static inline uint packetType(const packet *p){
	return p->typesize & 0xFF;
}
static inline void packetSet(packet *p, u8 ptype, u32 len){
	p->typesize = (u32)ptype | (len << 10);
}

void packetQueue         (packet *p, u8 ptype, u32 len, int c);
void packetQueueExcept   (packet *p, u8 ptype, u32 len, int c);
void packetQueueToServer (packet *p, u8 ptype, u32 len);
