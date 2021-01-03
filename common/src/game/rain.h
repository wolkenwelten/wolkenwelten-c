#pragma once
#include "../common.h"

#define RAIN_MAX (1<<16)
extern glRainDrop glRainDrops[RAIN_MAX];
extern   rainDrop   rainDrops[RAIN_MAX];
extern uint rainCount;

void rainNew(vec pos);
void rainUpdateAll();

void rainSendUpdate(uint c, uint i);
