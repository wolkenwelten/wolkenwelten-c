#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

extern entity entityList[1<<14];
extern int entityCount;

entity  *entityNew       (float x, float y, float z , float yaw, float pitch, float roll);
void     entityFree      (entity *e);
void     entityDraw      (entity *e);
void     entityDrawAll   ();
void     entityUpdateAll ();
