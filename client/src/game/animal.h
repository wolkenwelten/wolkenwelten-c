#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

void animalDrawAll        ();
void animalSyncFromServer (const packet *p);
void animalGotHitPacket   (const packet *p);
