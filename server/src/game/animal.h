#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern int animalCount;

animal  *animalNew        (float x, float y, float z , int type);
void     animalFree       (animal *e);
void     animalUpdateAll  ();
void     animalThinkAll   ();
void     animalSyncPlayer (int c);
uint8_t *animalLoad       (uint8_t *b);
uint8_t *animalSaveChungus(chungus *c,uint8_t *b);
void     animalDelChungus (chungus *c);
