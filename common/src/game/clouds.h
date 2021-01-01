#pragma once
#include "../../../common/src/common.h"

extern u8  cloudTex[256][256];
extern u8  cloudDensityMin;
extern vec cloudOff;
extern vec windVel,windGVel;

void cloudsInit();
void cloudsUpdateAll();
void cloudsSetWind(const vec ngv);
void cloudsSetDensity(u8 gd);

void cloudsRecvUpdate(const packet *p);
void cloudsSendUpdate(uint c);
