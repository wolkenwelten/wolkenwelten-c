#pragma once
#include "../../../common/src/common.h"

extern entity entityList[1<<14];
extern int entityCount;

entity  *entityNew       (float x, float y, float z, float yaw, float pitch, float roll);
void     entityFree      (entity *e);
void     entityReset     (entity *e);
void     entitySetPos    (entity *e, float  x, float  y, float  z);
void     entitySetVel    (entity *e, float vx, float vy, float vz);
void     entitySetRot    (entity *e, float yaw, float pitch, float roll);
uint32_t entityCollision (float cx, float cy, float cz);
int      entityUpdate    (entity *e);
void     entityDraw      (entity *e);
void     entityDrawAll   ();
void     entityUpdateAll ();
