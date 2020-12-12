#pragma once
#include "../../../common/src/common.h"

typedef struct {
	entity *ent;
	int ticksLeft;
	float pwr,cluster,clusterPwr;
} grenade;

extern grenade grenadeList[512];
extern uint    grenadeCount;

void     explode                 (const vec pos, float pw, int style);
void     grenadeUpdateFromServer (const packet *p);

grenade *grenadeGetByBeing       (being b);
being    grenadeGetBeing         (const grenade *g);

void     grenadeNew              (const vec pos, const vec rot, float pwr, int cluster, float clusterPwr);
void     grenadeUpdateAll        ();
