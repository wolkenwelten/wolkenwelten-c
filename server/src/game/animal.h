#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

animal     *animalNew         (const vec pos ,int type);
void        animalUpdateAll   ();
void        animalThinkAll    ();
uint        animalSyncPlayer  (int c, uint offset);
void        animalDelChungus  (const chungus *c);
void        animalDmgPacket   (int c, const packet *p);
