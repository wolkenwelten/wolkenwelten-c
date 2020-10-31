#pragma once
#include "../common.h"

extern rope ropeList[128];
extern uint ropeCount;

rope *ropeNew       (being a, being b);
void  ropeFree      (rope *r);
float ropeLength    (const rope *r);
void  ropeUpdateAll ();
