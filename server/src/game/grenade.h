#pragma once
#include "../../../common/src/common.h"

void explode             (const vec pos, float pwr, int style);
void grenadeNewP         (const packet *p);
void grenadeUpdate       ();
void grenadeUpdatePlayer (uint c);
void beamblastNewP       (uint c, const packet *p);
