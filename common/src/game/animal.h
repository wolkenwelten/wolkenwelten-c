#pragma once
#include "../../../common/src/common.h"

extern animal  animalList[1<<10];
extern uint    animalCount;

animal     *animalNew          (const vec pos , int type, int gender);
void        animalDel          (uint i);
void        animalReset        (      animal *e);
int         animalUpdate       (      animal *e);
float       animalDistance     (const animal *e,const character *c);
const char *animalGetStateName (const animal *e);
int         animalGetMaxHealth (const animal *e);

animal *animalGetByBeing(being b);
being   animalGetBeing  (const animal *h);
