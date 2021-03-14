#pragma once
#include <stdbool.h>

#if defined(__arm__) || defined(__aarch64__) || defined(__EMSCRIPTEN__)
	#define WOLKENWELTEN__GL_ES 1
#endif


#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#define WOLKENWELTEN__GL_USE_APPLE
#elif defined(__EMSCRIPTEN__) || defined(__HAIKU__)
	#include "GL/glew.h"
	#define WOLKENWELTEN__GL_USE_GLEW
#else
	#include "gl3w.h"
	#define WOLKENWELTEN__GL_USE_GL3W
#endif


#ifdef WOLKENWELTEN__GL_ES
	extern bool glIsMultiDrawAvailable;
#else
	static const bool glIsMultiDrawAvailable = true;
#endif

bool glInitialize();
