#pragma once
#include "../common.h"

extern rope ropeList[512];

int   ropeNewID     ();
rope *ropeNew       (being a, being b, u32 flags);
rope *ropeGetByID   (uint i);
uint  ropeGetID     (const rope *r);
void  ropeFree      (rope *r);
float ropeGetLength (const rope *r);
void  ropeSetLength (rope *r, float l);
void  ropeUpdateAll ();
int   ropeGetClient (uint i);
void  ropeDelBeing  (const being t);
