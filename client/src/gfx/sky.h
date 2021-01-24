#pragma once
#include "../../../common/src/common.h"

extern float sunAngle;
extern float skyBrightness;
extern float worldBrightness;

void initSky();
void renderSky(const character *cam);
