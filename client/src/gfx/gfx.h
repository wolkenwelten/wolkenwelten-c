#pragma once
#include "../../../common/src/common.h"

extern float matProjection[16], matView[16];
extern int screenWidth;
extern int screenHeight;
extern size_t vboTrisCount;

void initGL();
void renderWorld(character *cam);
void renderFrame();
void renderMenuFrame();
