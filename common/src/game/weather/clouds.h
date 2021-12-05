#pragma once
#include "../../../../common/src/common.h"

extern u8  cloudTex[256][256];
extern u8  cloudDensityMin,cloudGDensityMin;
extern vec cloudOff;

void cloudsInit();

void cloudsSetDensity(u8 gd);
bool isInClouds(const vec p);
