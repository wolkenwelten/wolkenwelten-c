#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

animal  *animalNew            (const vec pos, uint type);
void     animalDraw           (const animal *e);
void     animalDrawAll        ();
void     animalUpdateAll      ();
void     animalSyncFromServer (const packet *p);
