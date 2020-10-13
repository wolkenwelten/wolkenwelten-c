#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

void animalDrawAll        ();
void animalUpdateAll      ();
int  animalHitCheck       (const vec pos, float mdd, int dmg, int cause, uint iteration);
void animalSyncFromServer (const packet *p);
void animalGotHitPacket   (const packet *p);
