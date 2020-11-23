#pragma once
#include "../../../common/src/common.h"

typedef struct {
	u16 x,y,z;
	i16 strength;
	i16 blockDmg;
} fire;

extern fire fireList[1<<16];
extern uint fireCount;

void fireNew          (u16 x, u16 y, u16 z, i16 strength);
fire *fireGetAtPos    (u16 x,u16 y, u16 z);
void fireBox          (int x, int y, int z, int w, int h, int d);
void fireBoxExtinguish(int x, int y, int z, int w, int h, int d);
void fireSendUpdate   (uint c, uint i);
