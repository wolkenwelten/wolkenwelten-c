#pragma once
#include "../../../../common/src/common.h"
#include "../../../../common/src/game/character.h"

void  characterFireHook       (      character *c);
float characterCanHookHit     (const character *c);
void  characterAddHookLength  (      character *c,float d);
float characterGetHookLength  (const character *c);
float characterGetRopeLength  (const character *c);
void  characterFreeHook       (      character *c);
void  characterUpdateHook     (      character *c);
