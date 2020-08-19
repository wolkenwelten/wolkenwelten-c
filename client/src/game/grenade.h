#pragma once
#include "../../../common/src/common.h"

void grenadeExplode(float x, float y, float z, float pw, int style);
void grenadeNew(character *ent, float pwr);
void grenadeUpdate();
void grenadeUpdateFromServer(packet *p);
void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult);
