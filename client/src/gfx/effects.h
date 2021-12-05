#pragma once
#include "../../../common/src/common.h"

void fxExplosionBomb    (const vec pos,float pw);
void fxGrenadeTrail     (const vec pos,float pw);
void fxExplosionBlaster (const vec pos,float pw);
void fxBeamBlaster      (const vec pa,const vec pb, float beamSize, float damageMultiplier);
void fxBlockBreak       (const vec pos, blockId b, u8 cause);
void fxBlockMine        (const vec pos, int dmg, uchar b);
void fxBleeding         (const vec pos, being victim, i16 dmg, blockId cause);
void fxAnimalDiedPacket (const packet *p);
void fxProjectileHit    (const packet *p);
void fxRainDrop         (const vec pos);
void fxSnowDrop         (const vec pos);
