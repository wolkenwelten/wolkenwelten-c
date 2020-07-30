#pragma once
#include "../game/entity.h"
#include "../../../common/src/network/packet.h"

void explode             (float x, float y, float z, float pwr, int style);
void grenadeNew          (packet *p);
void grenadeUpdate       ();
void grenadeUpdatePlayer (int c);
void beamblast           (int c, const packet *p);
