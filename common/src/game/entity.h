#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

void   entityReset            (      entity *e);
float  entityDistance         (const entity *e,const character *c);
int    entityUpdate           (      entity *e);
void   entityUpdateCurChungus (      entity *e);
u32    entityCollision        (const vec c);

uint   lineOfSightBlockCount  (const vec a, const vec b, uint maxB);
