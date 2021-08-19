#pragma once
#include <stdbool.h>

#if defined(__arm__) || defined(__aarch64__) || defined(__EMSCRIPTEN__)
	#define WOLKENWELTEN__GL_ES 1
#endif

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#define WOLKENWELTEN__GL_USE_APPLE
#elif defined (__MINGW32__) || defined (__linux__) || defined(__NetBSD__) || defined(__OpenBSD__)|| defined(__FreeBSD__) || defined(__DragonflyBSD__)
	#include "gl3w.h"
	#define WOLKENWELTEN__GL_USE_GL3W
#else
	#include "GL/glew.h"
	#define WOLKENWELTEN__GL_USE_GLEW
#endif

bool glInitialize();
