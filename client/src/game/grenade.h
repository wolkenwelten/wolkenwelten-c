#pragma once
#include "../game/character.h"
#include "../network/packet.h"

void grenadeExplode(float x, float y, float z, float pw, int style);
void grenadeNew(character *ent, float pwr);
void grenadeUpdate();
void grenadeUpdateFromServer(packetMedium *p);
void beamblast(character *ent, float pwr, int hitsLeft);
