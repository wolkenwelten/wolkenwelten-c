#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/grenade.h"

void explode             (const vec pos, float pwr, int style);
void grenadeNewP         (const packet *p);
void grenadeUpdate       ();
void grenadeUpdatePlayer (u8 c);
