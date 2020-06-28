#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	float  x, y, z;
	float vx,vy,vz;
	float yaw,pitch,roll;
	float yoff;

	bool falling;
	bool noClip;
	bool updated;
	bool collide;
	bool noRepulsion;

	void *nextFree;
} entity;

extern entity entityList[1<<16];
extern int entityCount;

entity  *entityNew       (float x, float y, float z, float yaw, float pitch, float roll);
void     entityFree      (entity *e);
void     entityReset     (entity *e);
void     entitySetPos    (entity *e, float  x, float  y, float  z);
void     entitySetVel    (entity *e, float vx, float vy, float vz);
void     entitySetRot    (entity *e, float yaw, float pitch, float roll);
uint32_t entityCollision (entity *e, float cx, float cy, float cz, float wd);
int      entityUpdate    (entity *e);
void     entityUpdateAll ();
