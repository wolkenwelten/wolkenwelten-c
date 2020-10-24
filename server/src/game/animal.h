#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<10];
extern uint animalCount;

animal     *animalNew              (const vec pos ,int type);
void        animalUpdateAll        ();
void        animalThinkAll         ();
uint        animalSyncPlayer       (u8 c, uint offset);
void        animalDelChungus       (const chungus *c);
void        animalDmgPacket        (u8 c, const packet *p);
void        animalIntro            (u8 c);
float       animalClosestAnimal    (const animal *e, animal **cAnim, int typeFilter, uint flagsMask, uint flagsCompare);
float       animalClosestPlayer    (const animal *e, character **cChar);
void        animalCheckSuffocation (animal *e);
void        animalRDie             (animal *e);
void        animalRHit             (animal *e);
