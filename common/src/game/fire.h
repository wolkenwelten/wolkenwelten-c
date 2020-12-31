#pragma once
#include "../../../common/src/common.h"

extern fire fireList[1<<14];
extern uint fireCount;

void fireNew           (u16 x, u16 y, u16 z, i16 strength);
void fireDel           (uint i);
fire *fireGetAtPos     (u16 x,u16 y, u16 z);
void fireBox           (int x, int y, int z, int w, int h, int d, int strength);
void fireBoxExtinguish (int x, int y, int z, int w, int h, int d, int strength);
void fireSendUpdate    (uint c, uint i);

fire *fireGetByBeing   (being b);
being fireGetBeing     (const fire *f);
int fireHitCheck       (const vec pos, float mdd, int dmg, int cause, u16 iteration, being source);
