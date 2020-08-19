#pragma once
#include "../../../common/src/common.h"

void explode             (float x, float y, float z, float pwr, int style);
void grenadeNewP         (packet *p);
void grenadeUpdate       ();
void grenadeUpdatePlayer (int c);
void beamblastNewP       (int c, const packet *p);
