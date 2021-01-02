#pragma once
#include "../../../common/src/common.h"

extern u8  cloudTex[256][256];
extern u8  cloudDensityMin,rainDuration;
extern vec cloudOff;
extern vec windVel,windGVel;

void weatherInit();
void weatherUpdateAll();
void weatherDoRain();
void cloudsSetWind(const vec ngv);
void cloudsSetDensity(u8 gd);
void weatherSetRainDuration(u8 dur);

void weatherRecvUpdate(const packet *p);
void weatherSendUpdate(uint c);
