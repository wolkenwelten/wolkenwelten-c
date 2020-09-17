#pragma once
#include "../../../common/src/common.h"

void explode             (const vec pos, float pwr, int style);
void grenadeNewP         (const packet *p);
void grenadeUpdate       ();
void grenadeUpdatePlayer (int c);
void beamblastNewP       (int c, const packet *p);
