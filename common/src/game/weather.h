#pragma once
#include "../../../common/src/common.h"

#define RAIN_MAX (1<<16)
extern glRainDrop glRainDrops[RAIN_MAX+4];
extern   rainDrop   rainDrops[RAIN_MAX+4];
extern uint rainCount;

extern u8  cloudTex[256][256];
extern u8  cloudDensityMin,cloudGDensityMin,rainIntensity;
extern vec cloudOff;
extern vec windVel,windGVel;

void weatherInit();
void weatherUpdateAll();
void weatherDoRain();
void cloudsSetWind(const vec ngv);
void cloudsSetDensity(u8 gd);
void weatherSetRainDuration(u8 dur);

bool isInClouds(const vec p);

void rainNew(vec pos);
void rainUpdateAll();

void rainSendUpdate(uint c, uint i);
void weatherRecvUpdate(const packet *p);
void weatherSendUpdate(uint c);
