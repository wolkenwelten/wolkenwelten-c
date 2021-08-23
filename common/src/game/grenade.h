#pragma once
#include "../../../common/src/common.h"

extern grenade grenadeList[512];
extern uint    grenadeCount;

void     explode                 (const vec pos, float pw, int style);
void     grenadeUpdateFromServer (const packet *p);

grenade *grenadeGetByBeing       (being b);
being    grenadeGetBeing         (const grenade *g);

void     grenadeNew              (const vec pos, const vec rot, float pwr, int cluster, float clusterPwr);
void     grenadeUpdateAll        ();

void     explode                 (const vec pos, float pw, int style);
