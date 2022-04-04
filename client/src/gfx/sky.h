#pragma once
#include "../../../common/src/common.h"

extern float sunAngle;
extern float skyBrightness;

void initSky();
void renderSky(const character *cam);
