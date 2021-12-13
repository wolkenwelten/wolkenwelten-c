#pragma once
#include "../../../../common/src/common.h"

void lightningStrike(u16 lx, u16 ly, u16 lz, u16 tx, u16 ty, u16 tz, u16 seed, void (*fun)(const vec, const vec, int));
void lightningStrikeRec(const vec a, const vec b, uint stepsLeft, int size, bool branches, void (*fun)(const vec, const vec, int));
