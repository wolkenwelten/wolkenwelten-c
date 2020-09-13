#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

animal  *animalNew            (float x, float y, float z , int type);
void     animalDraw           (animal *e);
void     animalDrawAll        ();
void     animalUpdateAll      ();
void     animalSyncFromServer (packet *p);
