#pragma once
#include <stdbool.h>

#if defined(__arm__) || defined(__aarch64__) || defined(__EMSCRIPTEN__)
	#define WOLKENWELTEN__GL_ES 1
#endif

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#define WOLKENWELTEN__GL_USE_APPLE
#else
	#include "gl3w.h"
	#define WOLKENWELTEN__GL_USE_GL3W
#endif

#ifdef WOLKENWELTEN__GL_ES
extern bool glIsMultiDrawAvailable;
#else
#define glIsMultiDrawAvailable (true)
#endif

extern bool glIsBaseInstanceAvailable;
extern bool glIsMultiDrawIndirectAvailable;

extern bool glIsDebugAvailable;

bool glInitialize();

#ifdef __APPLE__
	#define gfxGroupStart(name) while(false){}
	#define gfxGroupEnd() while(false){}
	#define glObjectLabel(a,b,c,d) (void)d;
#else
	#define gfxGroupStart(name) if(glIsDebugAvailable){glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);}
	#define gfxGroupEnd() if(glIsDebugAvailable){glPopDebugGroup();}
#endif
