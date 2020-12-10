#include "../gfx/shader.h"
#include "../gfx/gl.h"
#include "../tmp/assets.h"

#include <stdio.h>
#include <string.h>

shader shaderList[8];
int    shaderCount = 0;
unsigned int activeProgram = 0;
shader *sMesh;
shader *sShadow;
shader *sBlockMesh;
shader *sParticle;
shader *sTextMesh;
shader *sCloud;

void shaderInit(){
	sMesh      = shaderNew((const char *)src_shader_meshShaderVS_glsl_data,     (const char *)src_shader_meshShaderFS_glsl_data,     0x3);
	sShadow    = shaderNew((const char *)src_shader_shadowShaderVS_glsl_data,   (const char *)src_shader_shadowShaderFS_glsl_data,   0x3);
	sBlockMesh = shaderNew((const char *)src_shader_blockShaderVS_glsl_data,    (const char *)src_shader_blockShaderFS_glsl_data,    0x3);
	sParticle  = shaderNew((const char *)src_shader_particleShaderVS_glsl_data, (const char *)src_shader_particleShaderFS_glsl_data, 0xD);
	sTextMesh  = shaderNew((const char *)src_shader_textShaderVS_glsl_data,     (const char *)src_shader_textShaderFS_glsl_data,     0x7);
	sCloud     = shaderNew((const char *)src_shader_cloudShaderVS_glsl_data,    (const char *)src_shader_cloudShaderFS_glsl_data,    0x5);
}

void shaderPrintLog(unsigned int obj, const char *msg, const char *src){
	int infologLength = 0;
	int maxLength;

	if(glIsShader(obj)){
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	} else {
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	}
	char infoLog[maxLength];

	if (glIsShader(obj)){
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
	}else{
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
	}

	if (infologLength > 0){
		printf("------------------\n%s\n\n%s\n\nSrc:\n%s\n\n------------------",msg,infoLog,src);
	}
}

void shaderCompile(shader *s){
	s->pID = glCreateProgram();
	static char buf[8192];
	char *bufp[2] = {(char *)&buf,0};

	s->vsID = glCreateShader(GL_VERTEX_SHADER);
	memset(buf,0,sizeof(buf));

	snprintf(buf,sizeof(buf)-1,"#version 130\n\n%s",s->vss);

	glShaderSource(s->vsID,1,(const GLchar **)&bufp,NULL);
	glCompileShader(s->vsID);
	shaderPrintLog(s->vsID,"Vertex Shader",buf);
	glAttachShader(s->pID,s->vsID);

	s->fsID = glCreateShader(GL_FRAGMENT_SHADER);
	memset(buf,0,sizeof(buf));

	#ifdef __APPLE__
		snprintf(buf,sizeof(buf)-1,"#version 120\n\n%s",s->fss);
	#else
		snprintf(buf,sizeof(buf)-1,"#version 100\nprecision mediump float;\n%s",s->fss);
	#endif


	glShaderSource(s->fsID,1,(const GLchar **)&bufp,NULL);
	glCompileShader(s->fsID);
	shaderPrintLog(s->fsID,"Fragment Shader",buf);
	glAttachShader(s->pID,s->fsID);

	if(s->attrMask & 0x1){
		glBindAttribLocation(s->pID,0,"pos");
	}
	if(s->attrMask & 0x2){
		glBindAttribLocation(s->pID,1,"tex");
	}
	if(s->attrMask & 0x4){
		glBindAttribLocation(s->pID,2,"color");
	}
	if(s->attrMask & 0x8){
		glBindAttribLocation(s->pID,3,"size");
	}
	glLinkProgram(s->pID);
	shaderPrintLog(s->pID,"Program",buf);

	s->lMVP        = glGetUniformLocation(s->pID,"matMVP");
	s->lAlpha      = glGetUniformLocation(s->pID,"colorAlpha");
	s->lBrightness = glGetUniformLocation(s->pID,"colorBrightness");
	s->lTransform  = glGetUniformLocation(s->pID,"transPos");
	s->lTex        = glGetUniformLocation(s->pID,"curTex");
}

shader *shaderNew(const char *vss,const char *fss,unsigned int attrMask){
	shader *s = &shaderList[shaderCount++];
	s->vss      = (char *)vss;
	s->fss      = (char *)fss;

	s->pID      = 0;
	s->vsID     = 0;
	s->fsID     = 0;

	s->attrMask = attrMask;
	s->lMVP     = -1;
	s->lAlpha   = -1;
	s->lTex     = -1;

	shaderCompile(s);
	return s;
}

void shaderFree(){
	for(int i=0;i<shaderCount;i++){
		if(shaderList[i].pID){
			glDeleteProgram(shaderList[i].pID);
		}
		if(shaderList[i].vsID){
			glDeleteShader (shaderList[i].vsID);
		}
		if(shaderList[i].fsID){
			glDeleteShader (shaderList[i].fsID);
		}
	}
}

void shaderBind(shader *s){
	if(activeProgram == s->pID){return;}
	glUseProgram(s->pID);
}

void shaderMatrix(shader *s, float mvp[16]){
	if(s->lMVP == -1){return;}
	glUniformMatrix4fv(s->lMVP,1,GL_FALSE,mvp);
}

void shaderAlpha(shader *s, float alpha){
	if(s->lAlpha == -1){return;}
	glUniform1f(s->lAlpha,alpha);
}

void shaderBrightness(shader *s, float v){
	if(s->lBrightness == -1){return;}
	glUniform1f(s->lBrightness,v);
}

void shaderTransform(shader *s,float x,float y,float z){
	if(s->lTransform == -1){ return; }
	glUniform3f(s->lTransform,x,y,z);
}

void shaderTex(shader *s, int t){
	if(s->lTex == -1){return;}
	glUniform1i(s->lTex,t);
}
