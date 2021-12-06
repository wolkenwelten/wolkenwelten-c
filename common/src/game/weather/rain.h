#pragma once
#include "../../../../common/src/common.h"

#define RAIN_MAX (1<<16)
extern rainDrop glRainDrops[RAIN_MAX];
extern rainDrop   rainDrops[RAIN_MAX];
extern uint rainCount;
extern u8 rainIntensity;

void rainInit();
void rainNew(vec pos);
void rainUpdateAll();
void weatherDoRain();
void rainSendUpdate(uint c, uint i);
void weatherSetRainIntensity(u8 intensity);
