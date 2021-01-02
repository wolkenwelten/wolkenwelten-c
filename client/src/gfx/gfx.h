#pragma once
#include "../../../common/src/common.h"

extern float  matProjection[16], matView[16];
extern int    screenWidth;
extern int    screenHeight;
extern size_t vboTrisCount;
extern vec    camShake;
extern float  gfxCurFOV;

extern float renderDistance,renderDistanceSquare;
extern float fadeoutDistance;
extern float fadeoutStartDistance;
extern float cloudFadeD;
extern float cloudMinD;
extern float cloudMaxD;

void initGL            ();
void setRenderDistance (float newRD);
void renderWorld       (const character *cam);
void renderFrame       ();
void renderMenuFrame   ();
