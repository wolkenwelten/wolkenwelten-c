#pragma once
#include "../../../common/src/common.h"

void fxExplosionBomb    (const vec pos,float pw);
void fxGrenadeTrail     (const vec pos,float pw);
void fxExplosionBlaster (const vec pos,float pw);
void fxBeamBlaster      (const vec pa,const vec pb, float beamSize, float damageMultiplier);
void fxBlockBreak       (const vec pos, uchar b);
void fxBlockMine        (const vec pos, int dmg, uchar b);
