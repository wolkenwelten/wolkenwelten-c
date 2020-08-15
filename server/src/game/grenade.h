#pragma once
#include "../game/entity.h"
#include "../../../common/src/network/packet.h"

void explode             (float x, float y, float z, float pwr, int style);
void grenadeNewP         (packet *p);
void grenadeUpdate       ();
void grenadeUpdatePlayer (int c);
void beamblastNewP       (int c, const packet *p);
