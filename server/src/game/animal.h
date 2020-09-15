#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

animal  *animalNew        (float x, float y, float z , int type);
void     animalUpdateAll  ();
void     animalThinkAll   ();
uint     animalSyncPlayer (int c, uint offset);
void    *animalLoad       (void *buf);
void    *animalSaveChungus(chungus *c,void *buf);
void     animalDelChungus (chungus *c);
