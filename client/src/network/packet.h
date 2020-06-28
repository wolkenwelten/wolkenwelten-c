#pragma once
#include "../voxel/chunk.h"
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
	uint16_t ptype;
	uint16_t target;

	union {
		float    f[3];
		int32_t  i[3];
		uint32_t u[3];
		uint8_t  c[3*4];
	} val;
} packetSmall;

typedef struct {
	uint16_t ptype;
	uint16_t target;

	union {
		float    f[7];
		int32_t  i[7];
		uint32_t u[7];
		uint8_t  c[7*4];
	} val;
} packetMedium;

typedef struct {
	uint16_t ptype;
	uint16_t target;

	union {
		float    f[31];
		int32_t  i[31];
		uint32_t u[31];
		uint8_t  c[31*4];
	} val;
} packetLarge;

typedef struct {
	uint16_t ptype;
	uint16_t target;

	union {
		float    f[1027];
		int32_t  i[1027];
		uint32_t u[1027];
		uint8_t  c[1027*4];
	} val;
} packetHuge;
#pragma pack(pop)

void packetQueueS(packetSmall  *p, uint16_t ptype);
void packetQueueM(packetMedium *p, uint16_t ptype);
void packetQueueL(packetLarge  *p, uint16_t ptype);
void packetQueueH(packetHuge   *p, uint16_t ptype);
int  packetLen(void *p);

void packetQueueChunk(chunk *chnk);
