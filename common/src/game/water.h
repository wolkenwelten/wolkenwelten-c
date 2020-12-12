#pragma once
#include "../../../common/src/common.h"

extern water waterList[1<<14];
extern uint  waterCount;

void   waterNew        (u16 x, u16 y, u16 z, i16 amount);
water *waterGetAtPos   (u16 x, u16 y, u16 z);
void   waterBox        (u16 x, u16 y, u16 z, i16 w, i16 h, i16 d, i16 amount);
void   waterSendUpdate (uint c, uint i);

water *waterGetByBeing (being b);
being  waterGetBeing   (const water *w);
