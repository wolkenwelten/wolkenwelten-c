#pragma once
#include "../../../common/src/common.h"
#include "gl.h"

extern bool   queueScreenshot;
extern bool   gfxUseSubData;
extern bool   gfxInitComplete;

extern float  matProjection[16], matSubBlockView[16], matView[16], matSkyProjection[16];
extern vec    subBlockViewOffset;
extern int    screenWidth;
extern int    screenHeight;
extern int    screenRefreshRate;
extern uint   frameRelaxedDeadline;
extern size_t vboTrisCount;
extern size_t drawCallCount;
extern vec    camShake;
extern float  gfxCurFOV;

extern float renderDistance,renderDistanceSquare;
extern float fadeoutDistance;
extern float fadeoutStartDistance;
extern float cloudFadeD;
extern float cloudMinD;
extern float cloudMaxD;

void initMenuBackground();
void initGL            ();
void setRenderDistance (float newRD);
void renderWorld       (const character *cam);
void renderFrame       ();
void renderMenuFrame   ();

GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)
