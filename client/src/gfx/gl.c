#include <string.h>
#include <stdio.h>
#include "gl.h"

#ifdef WOLKENWELTEN__GL_ES
bool glIsMultiDrawAvailable;
#endif

bool glIsBaseInstanceAvailable;
bool glIsMultiDrawIndirectAvailable;

bool glIsDebugAvailable;
uint16_t glDebugLabelMaxLen;

bool glHasExtension(const char *name){
	GLint numExt;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
	for(GLint i = 0;i < numExt;++i) {
		if(strcmp((const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i), name) == 0){ return true; }
	}
	return false;
}

bool glInitialize() {
	#ifdef WOLKENWELTEN__GL_USE_GL3W
	int errCode = gl3wInit();
	if(errCode){
		fprintf(stderr,"\nERROR: Can't initialize OpenGL 3.1 using GL3W, you might need a newer GPU/Driver.\nErrorcode: %u %s\n",errCode,errCode == GL3W_ERROR_OPENGL_VERSION ? "GL3W_ERROR_OPENGL_VERSION" : errCode == GL3W_ERROR_INIT ? "GL3W_ERROR_INIT" : "UNKNOWN" );
		return false;
	}
	#endif

	glIsDebugAvailable = glHasExtension("GL_KHR_debug");
	if(glIsDebugAvailable){
		GLint value;
		glGetIntegerv(GL_MAX_LABEL_LENGTH, &value);
		glDebugLabelMaxLen = value > 256 ? 256 : value;
	}
	else{
		glDebugLabelMaxLen = 0;
	}

	return true;
}
