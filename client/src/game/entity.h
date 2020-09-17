#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

extern entity entityList[1<<14];
extern int entityCount;

entity  *entityNew       (const vec pos, const vec rot);
void     entityFree      (entity *e);
void     entityDraw      (const entity *e);
void     entityDrawAll   ();
void     entityUpdateAll ();
