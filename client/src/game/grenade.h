#pragma once
#include "../../../common/src/common.h"

void grenadeExplode         (vec pos, float pw, int style);
void grenadeNew             (const character *ent, float pwr);
void grenadeUpdate          ();
void grenadeUpdateFromServer(const packet *p);
void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult);
