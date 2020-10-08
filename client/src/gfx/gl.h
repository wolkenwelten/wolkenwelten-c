#pragma once

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
	#include <SDL_opengles2.h>
	#define INITGLEXT() 0
#endif
