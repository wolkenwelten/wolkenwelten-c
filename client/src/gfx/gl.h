#pragma once

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#define INITGLEXT() 0
#else
	#include "gl3w.h"
	#define INITGLEXT() if(gl3wInit()){exit(3);}
#endif
