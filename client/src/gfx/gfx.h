#pragma once
#include "../../../common/src/common.h"

extern float  matProjection[16], matView[16];
extern int    screenWidth;
extern int    screenHeight;
extern size_t vboTrisCount;
extern vec    camShake;

void initGL          ();
void renderWorld     (const character *cam);
void renderFrame     ();
void renderMenuFrame ();
