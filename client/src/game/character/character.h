#pragma once
#include "../../../../common/src/common.h"
#include "../../../../common/src/game/character.h"

extern character *player;

void  characterDoPrimary       (      character *c);
void  characterThrow           (      character *c);
void  characterMineStop        (      character *c);
float characterMineProgress    (      character *c);
void  characterUpdate          (      character *c);
float characterFirstBlockDist  (const character *c);
void  characterItemDrop        (character *c, int i);
void  characterItemDropSingle  (character *c, int i);
int   characterHitCheck        (const vec pos, float mdd, int damage, int cause, u16 iteration, being source);
void  characterUpdateAll       ();
