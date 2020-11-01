#pragma once
#include "../common.h"

extern rope ropeList[128];
extern uint ropeCount;

rope *ropeNew       (being a, being b, u32 flags);
void  ropeFree      (rope *r);
float ropeGetLength (const rope *r);
void  ropeUpdateAll ();
