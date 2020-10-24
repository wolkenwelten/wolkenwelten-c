#pragma once
#include "../../../common/src/common.h"

void        animalReset        (      animal *e);
int         animalUpdate       (      animal *e);
float       animalDistance     (const animal *e,const character *c);
const char *animalGetStateName (const animal *e);
int         animalGetMaxHealth (const animal *e);
