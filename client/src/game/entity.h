#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

extern entity entityList[1<<14];
extern int entityCount;

void     entityDraw      (entity *e);
void     entityDrawAll   ();
