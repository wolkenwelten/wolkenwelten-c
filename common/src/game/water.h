#pragma once
#include "../../../common/src/common.h"

typedef struct {
	u16 x,y,z;
	i16 amount;
} water;

extern water waterList[1<<14];
extern uint  waterCount;

void waterNew        (u16 x, u16 y, u16 z, i16 amount);
water *waterGetAtPos (u16 x, u16 y, u16 z);
void waterBox        (u16 x, u16 y, u16 z, i16 w, i16 h, i16 d, i16 amount);
void waterSendUpdate (uint c, uint i);
