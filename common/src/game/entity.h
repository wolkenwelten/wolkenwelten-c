#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

void     entityReset     (      entity *e);
float    entityDistance  (const entity *e,const character *c);
int      entityUpdate    (      entity *e);
uint32_t entityCollision (const vec c);
