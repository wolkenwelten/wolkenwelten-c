/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../gfx/shader.h"
#include "../gfx/gl.h"

#include <stdio.h>
#include <string.h>

shader  shaderList[8];
int     shaderCount = 0;
uint    activeProgram = 0;
shader *sMesh;
shader *sShadow;
shader *sBlockMesh;
shader *sParticle;
shader *sRain;
shader *sTextMesh;
shader *sCloud;
shader *sBoundary;

extern u8 src_shader_boundaryShaderFS_glsl_data[];
extern u8 src_shader_boundaryShaderVS_glsl_data[];
extern u8 src_shader_blockShaderFS_glsl_data[];
extern u8 src_shader_blockShaderVS_glsl_data[];
extern u8 src_shader_cloudShaderFS_glsl_data[];
extern u8 src_shader_cloudShaderVS_glsl_data[];
extern u8 src_shader_meshShaderFS_glsl_data[];
extern u8 src_shader_meshShaderVS_glsl_data[];
extern u8 src_shader_particleShaderFS_glsl_data[];
extern u8 src_shader_particleShaderVS_glsl_data[];
extern u8 src_shader_rainShaderFS_glsl_data[];
extern u8 src_shader_rainShaderVS_glsl_data[];
extern u8 src_shader_shadowShaderFS_glsl_data[];
extern u8 src_shader_shadowShaderVS_glsl_data[];
extern u8 src_shader_textShaderFS_glsl_data[];
extern u8 src_shader_textShaderVS_glsl_data[];

void shaderInit(){
	sMesh      = shaderNew((const char *)src_shader_meshShaderVS_glsl_data,     (const char *)src_shader_meshShaderFS_glsl_data,     0x3);
	sShadow    = shaderNew((const char *)src_shader_shadowShaderVS_glsl_data,   (const char *)src_shader_shadowShaderFS_glsl_data,   0x3);
	sBlockMesh = shaderNew((const char *)src_shader_blockShaderVS_glsl_data,    (const char *)src_shader_blockShaderFS_glsl_data,    0x7);
	sParticle  = shaderNew((const char *)src_shader_particleShaderVS_glsl_data, (const char *)src_shader_particleShaderFS_glsl_data, 0x5);
	sRain      = shaderNew((const char *)src_shader_rainShaderVS_glsl_data,     (const char *)src_shader_rainShaderFS_glsl_data,     0x1);
	sTextMesh  = shaderNew((const char *)src_shader_textShaderVS_glsl_data,     (const char *)src_shader_textShaderFS_glsl_data,     0x7);
	sCloud     = shaderNew((const char *)src_shader_cloudShaderVS_glsl_data,    (const char *)src_shader_cloudShaderFS_glsl_data,    0x5);
	sBoundary  = shaderNew((const char *)src_shader_boundaryShaderVS_glsl_data, (const char *)src_shader_boundaryShaderFS_glsl_data, 0x5);
}

void shaderPrintLog(uint obj, const char *msg, const char *src){
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

void compileVertexShader(shader *s){
	char buf[1 << 14];
	char *bufp[2] = {(char *)&buf,0};

	s->vsID = glCreateShader(GL_VERTEX_SHADER);
	#ifdef WOLKENWELTEN__GL_ES
	snprintf(buf,sizeof(buf),"#version 300 es\n"
"precision mediump float;\n"
"precision mediump int;\n"
"\n"
"%s",s->vss);
	#elif defined(__APPLE__)
	snprintf(buf,sizeof(buf),"#version 330 core\n"
"precision mediump float;\n"
"precision mediump int;\n"
"\n"
"%s",s->vss);
	#else
	snprintf(buf,sizeof(buf),"#version 140\n"
"\n"
"%s",s->vss);
	#endif
	glShaderSource(s->vsID,1,(const GLchar **)&bufp,NULL);
	glCompileShader(s->vsID);
	shaderPrintLog(s->vsID,"Vertex Shader",buf);
	glAttachShader(s->pID,s->vsID);
}

void compileFragmentShader(shader *s){
	char buf[1 << 14];
	char *bufp[2] = {(char *)&buf,0};

	s->fsID = glCreateShader(GL_FRAGMENT_SHADER);
	#ifdef WOLKENWELTEN__GL_ES
	snprintf(buf,sizeof(buf),"#version 300 es\n"
"precision mediump float;\n"
"precision mediump int;\n"
"precision lowp sampler2DArray;\n"
"\n"
"%s",s->fss);
	#elif defined(__APPLE__)
	snprintf(buf,sizeof(buf),"#version 330 core\n"
"precision mediump float;\n"
"precision mediump int;\n"
"precision lowp sampler2DArray;\n"
"\n"
"%s",s->fss);
	#else
	snprintf(buf,sizeof(buf),"#version 130\n"
"\n"
"%s",s->fss);
	#endif

	glShaderSource(s->fsID,1,(const GLchar **)&bufp,NULL);
	glCompileShader(s->fsID);
	shaderPrintLog(s->fsID,"Fragment Shader",buf);
	glAttachShader(s->pID,s->fsID);
}

void shaderCompile(shader *s){
	s->pID = glCreateProgram();
	compileVertexShader(s);
	compileFragmentShader(s);

	if(s->attrMask & 0x1){glBindAttribLocation(s->pID,0,"pos");}
	if(s->attrMask & 0x2){glBindAttribLocation(s->pID,1,"tex");}
	if(s->attrMask & 0x4){glBindAttribLocation(s->pID,2,"color");}
	if(s->attrMask & 0x8){glBindAttribLocation(s->pID,3,"size");}

	glLinkProgram(s->pID);
	shaderPrintLog(s->pID,"Program","");

	s->lMVP        = glGetUniformLocation(s->pID,"matMVP");
	s->lColor      = glGetUniformLocation(s->pID,"inColor");
	s->lAlpha      = glGetUniformLocation(s->pID,"colorAlpha");
	s->lSideTint   = glGetUniformLocation(s->pID,"sideTint");
	s->lTransform  = glGetUniformLocation(s->pID,"transPos");
	s->lSizeMul    = glGetUniformLocation(s->pID,"sizeMul");
}

shader *shaderNew(const char *vss,const char *fss,uint attrMask){
	shader *s = &shaderList[shaderCount++];
	s->vss      = (char *)vss;
	s->fss      = (char *)fss;

	s->pID      = 0;
	s->vsID     = 0;
	s->fsID     = 0;

	s->attrMask = attrMask;
	s->lMVP     = -1;
	s->lColor   = -1;
	s->lAlpha   = -1;
	s->lSizeMul = -1;

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

void shaderColor(shader *s, float r, float g, float b, float a){
	if(s->lColor == -1){return;}
	glUniform4f(s->lColor,r,g,b,a);
}

void shaderSideTint(shader *s, const vec v){
	if(s->lSideTint == -1){return;}
	glUniform3f(s->lSideTint,v.x,v.y,v.z);
}

void shaderTransform(shader *s, float x, float y, float z){
	if(s->lTransform == -1){ return; }
	glUniform3f(s->lTransform,x,y,z);
}

void shaderSizeMul(shader *s, float sizeMul){
	if(s->lSizeMul == -1){return;}
	glUniform1f(s->lSizeMul,sizeMul);
}
