#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

entity  *entityNew       (float x, float y, float z, float yaw, float pitch, float roll);
void     entityFree      (entity *e);
void     entityReset     (entity *e);
float    entityDistance  (entity *e, character *c);
int      entityUpdate    (entity *e);
uint32_t entityCollision (float cx, float cy, float cz);
void     entityUpdateAll ();
