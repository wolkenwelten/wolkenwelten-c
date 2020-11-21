#pragma once
#include "../../../common/src/common.h"

typedef struct {
	u16 x,y,z;
	i16 strength;
} fire;

extern fire fireList[1<<16];
extern uint fireCount;

void fireNew       (const vec pos, u16 strength);
void fireBox       (int x, int y, int z, int w, int h, int d);
void fireSendUpdate(uint c, uint i);
