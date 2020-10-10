#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

void     animalDrawAll        ();
void     animalUpdateAll      ();
int      animalBlastHitCheck  (const vec pos, float beamSize, float damageMultiplier, uint iteration);
void     animalSyncFromServer (const packet *p);
