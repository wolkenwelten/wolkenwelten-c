#pragma once
#include "../../../common/src/common.h"

void singleBeamblast(character *ent, const vec start, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft);
void beamblast      (character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult);
