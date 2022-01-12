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
extern uint16_t glDebugLabelMaxLen;

bool glInitialize();

#ifdef __APPLE__
	#define gfxGroupStart(name) while(false){}
	#define gfxGroupEnd() while(false){}
	#define gfxObjectLabel(a,b,...) while(false){};
#else
	#define gfxGroupStart(name) if(glIsDebugAvailable){glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);}
	#define gfxGroupEnd() if(glIsDebugAvailable){glPopDebugGroup();}

	#include <string.h>
	#define gfxObjectLabel(type, id, ...) if(glIsDebugAvailable){\
		char fullname[256];\
		if(type==GL_VERTEX_ARRAY){glBindVertexArray(id);}\
		if(type==GL_BUFFER){glBindBuffer(GL_ARRAY_BUFFER,id);}\
		/*if(type==GL_PROGRAM){glUseProgram(id);}*/\
		snprintf(fullname,glDebugLabelMaxLen,__VA_ARGS__);\
		glObjectLabel(type,id,-1,fullname);\
	}
#endif
