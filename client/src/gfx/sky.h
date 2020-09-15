#pragma once
#include "../../../common/src/common.h"

extern float sunAngle;

void initSky();
void renderSky(const character *cam);
void cloudsDraw(const character *cam);
