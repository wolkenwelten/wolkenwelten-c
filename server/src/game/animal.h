#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

animal     *animalNew         (const vec pos ,int type);
void        animalUpdateAll   ();
void        animalThinkAll    ();
uint        animalSyncPlayer  (int c, uint offset);
const void *animalLoad        (const void *buf);
void       *animalSaveChungus (const chungus *c,void *buf);
void        animalDelChungus  (const chungus *c);
