#pragma once

#if defined(__arm__) || defined(__aarch64__) || defined(__EMSCRIPTEN__)
#define WOLKENWELTEN__GL_ES 1
#endif

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#define INITGLEXT() 0
#elif __EMSCRIPTEN__
	#include "GL/glew.h"
	#define INITGLEXT() glewExperimental=GL_TRUE; if(glewInit() != GLEW_OK){exit(3);}
#elif __HAIKU__
	#include "GL/glew.h"
	#define INITGLEXT() glewExperimental=GL_TRUE; if(glewInit() != GLEW_OK){exit(3);}
#else
	#include "gl3w.h"
	#define INITGLEXT() if(gl3wInit()){exit(3);}
#endif
