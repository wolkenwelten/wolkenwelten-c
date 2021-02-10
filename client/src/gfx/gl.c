#include <string.h>
#include <stdio.h>
#include "gl.h"

#ifdef WOLKENWELTEN__GL_ES
bool glIsMultiDrawAvailable;
#endif

bool glHasExtension(const char *name){
	GLint numExt;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
	for(GLint i = 0;i < numExt;++i) {
		if(strcmp((const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i), name) == 0){ return true; }
	}
	return false;
}

bool glInitialize() {
#ifdef WOLKENWELTEN__GL_USE_GLEW
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK){ return false; }
#endif
#ifdef WOLKENWELTEN__GL_USE_GL3W
	if(gl3wInit()){ return false; }
#endif

#ifdef WOLKENWELTEN__GL_ES
	glIsMultiDrawAvailable = glHasExtension("GL_EXT_multi_draw_arrays");
#endif

	return true;
}
