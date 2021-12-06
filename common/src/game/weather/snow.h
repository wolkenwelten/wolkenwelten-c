#pragma once
#include "../../../../common/src/common.h"

#define SNOW_MAX (1<<16)

extern snowDrop glSnowDrops[SNOW_MAX];
extern snowDrop   snowDrops[SNOW_MAX];
extern u8 snowIntensity;
extern uint snowCount;

void snowInit();
void snowNew(vec pos);
void snowSendUpdate(uint c, uint i);
void snowUpdateAll();
void weatherSetSnowIntensity(u8 intensity);
