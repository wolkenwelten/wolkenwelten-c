#pragma once
#include "../../../common/src/common.h"

void singleBeamblast(const vec start, const vec rot, float beamSize, float damageMultiplier, int hitsLeft);
void beamblast      (character *ent, float beamSize, float damageMultiplier, int hitsLeft);
