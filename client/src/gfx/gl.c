#include <string.h>
#include <stdio.h>
#include "gl.h"

bool glInitialize() {
	#ifdef WOLKENWELTEN__GL_USE_GLEW
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK){
		fprintf(stderr,"\nERROR: Can't initialize OpenGL 3.1 using GLEW, you might need a newer GPU/Driver.\n\n");
		return false;
	}
	#endif

	#ifdef WOLKENWELTEN__GL_USE_GL3W
	int errCode = gl3wInit();
	if(errCode){
		fprintf(stderr,"\nERROR: Can't initialize OpenGL 3.1 using GL3W, you might need a newer GPU/Driver.\nErrorcode: %u %s\n",errCode,errCode == GL3W_ERROR_OPENGL_VERSION ? "GL3W_ERROR_OPENGL_VERSION" : errCode == GL3W_ERROR_INIT ? "GL3W_ERROR_INIT" : "UNKNOWN" );
		return false;
	}
	#endif

	return true;
}
