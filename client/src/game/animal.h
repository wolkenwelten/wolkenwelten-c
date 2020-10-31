#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

void animalDrawAll        ();
void animalUpdateAll      ();
int  animalHitCheck       (const vec pos, float mdd, int dmg, int cause, u16 iteration);
void animalSyncFromServer (const packet *p);
void animalGotHitPacket   (const packet *p);
