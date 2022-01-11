#pragma once
#include "../../../common/src/common.h"

extern framebufferObject fboGame;
extern bool renderToFBO;

void fboInitGlobal ();
void fboInit       (framebufferObject *fbo, uint w, uint h);
void fboBind       (framebufferObject *fbo, uint w, uint h);
void fboBlit       (framebufferObject *fbo, uint x, uint y, uint w, uint h);
void fboFree       (framebufferObject *fbo);
void fboBindScreen ();
