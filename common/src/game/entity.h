#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

extern entity entityList[1<<14];
extern uint   entityCount;

entity  *entityNew              (const vec pos, const vec rot);
void     entityFree             (entity *e);
void     entityUpdateAll        ();
void     entityReset            (      entity *e);
float    entityDistance         (const entity *e,const character *c);
int      entityUpdate           (      entity *e);
void     entityUpdateCurChungus (      entity *e);
u32      entityCollision        (const vec c);

uint   lineOfSightBlockCount  (const vec a, const vec b, uint maxB);
