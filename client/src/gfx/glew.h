#pragma once

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#define INITGLEW() 0
#else
	#include <GL/glew.h>
	#define INITGLEW() glewExperimental=GL_TRUE; if(glewInit() != GLEW_OK){exit(3);}
#endif
