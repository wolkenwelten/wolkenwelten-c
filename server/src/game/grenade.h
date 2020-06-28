#pragma once
#include "../game/entity.h"
#include "../network/packet.h"

void explode             (float x, float y, float z, float pwr, int style);
void grenadeNew          (packetMedium *p);
void grenadeUpdate       ();
void grenadeUpdatePlayer (int c);
void beamblast           (int c, const packetMedium *p);
