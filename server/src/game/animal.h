#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern int animalCount;

animal  *animalNew       (float x, float y, float z , int type);
void     animalFree      (animal *e);
void     animalUpdateAll ();
