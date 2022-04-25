#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/entity.h"

extern entity entityList[1<<14];
extern uint   entityCount;
extern uint   entityMax;

entity  *entityNew              (const vec pos, const vec rot, float weight);
void     entityFree             (      entity *e);
void     entityReset            (      entity *e);
float    entityDistance         (const entity *e,const character *c);
int      entityUpdate           (      entity *e);
void     entityUpdateCurChungus (      entity *e);
u32      entityCollision        (const vec c);
u8       entityCollisionBlock   (const vec pos, vec *retPos);
void     entityUpdateAll        ();
i64      entityID               (const entity *e);
entity  *entityGetByID          (i64 id);
entity  *entityGetByBeing       (being b);
being    entityGetBeing         (const entity *e);
float    blockRepulsion         (const vec pos, float *vel, float weight, u8 (*colFunc)(const vec,vec *));

uint   lineOfSightBlockCount    (const vec a, const vec b, uint maxB);
