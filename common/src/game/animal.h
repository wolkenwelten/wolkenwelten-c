#pragma once
#include "../../../common/src/common.h"

extern animal  animalList[1<<10];
extern uint    animalCount;
extern uint    animalUsedCount;

animal     *animalNew              (const vec pos , int type, int gender);
void        animalDel              (uint i);
void        animalReset            (      animal *e);
float       animalDistance         (const animal *e,const character *c);
const char *animalGetStateName     (const animal *e);
int         animalGetMaxHealth     (const animal *e);
int         animalUpdate           (      animal *e);
void        animalThink            (      animal *e);
float       animalClosestAnimal    (const animal *e, animal **cAnim, int typeFilter, uint flagsMask, uint flagsCompare);
float       animalClosestPlayer    (const animal *e, character **cChar);
void        animalCheckSuffocation (animal *e);
void        animalRDie             (animal *e);
void        animalRHit             (animal *e);
void        animalSync             (u8 c, u16 i);
void        animalDmgPacket        (u8 c, const packet *p);

void        animalUpdateAll        ();
void        animalThinkAll         ();
void        animalNeedsAll         ();


animal *animalGetByBeing(being b);
being   animalGetBeing  (const animal *h);
animal *animalClosest   (const vec pos, float maxDistance);
int     animalHitCheck  (const vec pos, float mdd, int dmg, int cause, u16 iteration);
