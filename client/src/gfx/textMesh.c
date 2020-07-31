#include "textMesh.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gfx/shader.h"
#include "../tmp/assets.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../gfx/glew.h"

char stringBuffer[256];
textMesh textMeshList[8];
int  textMeshCount = 0;
textMesh *textMeshFirstFree = NULL;

textMesh *textMeshNew(){
	textMesh *m = NULL;
	if(textMeshFirstFree != NULL){
		m = textMeshFirstFree;
		textMeshFirstFree = m->nextFree;
	}
	if(m == NULL){
		if(textMeshCount >= (int)(sizeof(textMeshList) / sizeof(textMesh))-1){
			fprintf(stderr,"textMeshList Overflow!\n");
			return NULL;
		}
		m = &textMeshList[textMeshCount++];
	}
	m->vboSize  = 0;
	m->vbo = m->dataCount = 0;
	m->sx = m->sy = 0;
	m->size     = 1;
	m->tex      = tGui;
	m->finished = 0;
	m->usage    = GL_STREAM_DRAW;

	return m;
}

void textMeshFree(textMesh *m){
	if(m->vbo){
		glDeleteBuffers(1,&m->vbo);
	}
	m->nextFree = textMeshFirstFree;
	textMeshFirstFree = m;
}

void textMeshEmpty(textMesh *m){
	m->dataCount = 0;
	m->sx        = 0;
	m->sy        = 0;
	m->size      = 1;
	m->finished  = 0;
}

void textMeshAddVert(textMesh *m, int16_t x, int16_t y, int16_t u, int16_t v, uint32_t rgba){
	m->dataBuffer[m->dataCount++] = (vertex2D){x,y,u,v,rgba};
}

void textMeshDraw(textMesh *m){
	if(!m->vbo) {
		glGenBuffers(1,&m->vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	if(!m->finished){
		if(m->dataCount <= m->vboSize){
			glBufferSubData(GL_ARRAY_BUFFER, 0, m->dataCount*sizeof(vertex2D), m->dataBuffer);
		}else{
			glBufferData(GL_ARRAY_BUFFER, m->dataCount*sizeof(vertex2D), m->dataBuffer, m->usage);
			m->vboSize = m->dataCount;
		}
		m->finished = 1;
	}
	textureBind(m->tex);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_SHORT        , GL_FALSE, sizeof(vertex2D), (void *)(((char *)&m->dataBuffer[0].x)    - ((char *)m->dataBuffer)));
	glVertexAttribPointer(1, 2, GL_SHORT        , GL_FALSE, sizeof(vertex2D), (void *)(((char *)&m->dataBuffer[0].u)    - ((char *)m->dataBuffer)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex2D), (void *)(((char *)&m->dataBuffer[0].rgba) - ((char *)m->dataBuffer)));
	glDrawArrays(GL_TRIANGLES,0,m->dataCount);

	vboTrisCount += m->dataCount/3;
}

void textMeshAddGlyph(textMesh *m, int x, int y, int size, unsigned char c){
	float gx;
	float gy;
	int   glyphWidth = 8*size;
	float glyphSize  = 1.f / 64.f;

	if(c==0)  {return;}
	if(c==' '){return;}
	if(c>127) {return;}

	if(size == 1){
		glyphSize = 1.f / 128.f;
	}

	gx = ((float)(c&0x0f))*glyphSize;
	gy = ((float)(63-((c>>4)&0x0f)))*glyphSize;
	if(size == 1){
		gy = ((float)(15-((c>>4)&0x0f)))*glyphSize + 24.f/32.f;
	}

	textMeshAddVert( m, x           , y           , ((gx          )*128.f), ((gy          )*128.f),0xFFFFFFFF);
	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, ((gx+glyphSize)*128.f), ((gy+glyphSize)*128.f),0xFFFFFFFF);
	textMeshAddVert( m, x+glyphWidth, y           , ((gx+glyphSize)*128.f), ((gy          )*128.f),0xFFFFFFFF);

	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, ((gx+glyphSize)*128.f), ((gy+glyphSize)*128.f),0xFFFFFFFF);
	textMeshAddVert( m, x           , y           , ((gx          )*128.f), ((gy          )*128.f),0xFFFFFFFF);
	textMeshAddVert( m, x           , y+glyphWidth, ((gx          )*128.f), ((gy+glyphSize)*128.f),0xFFFFFFFF);
}

void textMeshAddStrPS(textMesh *m, int x, int y, int size, const char *str){
	const int glyphWidth = 8*size;
	const int lineHeight = 10*size;

	while(*str != 0){
		if(x+glyphWidth > screenWidth){
			x = m->sx;
			y += lineHeight;
			continue;
		}
		if(*str == '\n'){
			x = m->sx;
			m->sy += lineHeight;
			str++;
			continue;
		}
		if(*str == '\r'){
			str++;
			continue;
		}
		if(*str == '\t'){
			str++;
			x = (((x - m->sx) / (glyphWidth*4) ) + 1 ) * (glyphWidth*4);
			continue;
		}
		textMeshAddGlyph(m,x,y,size,*str);
		x += glyphWidth;
		str++;
	}
}

void textMeshAddString(textMesh *m, const char *str){
	textMeshAddStrPS(m,m->sx,m->sy,m->size,str);
}

void textMeshPrintfPS(textMesh *m, int x, int y, int size, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsprintf(stringBuffer,format,ap);
	va_end(ap);
	if(stringBuffer[0]==0){return;}

	textMeshAddStrPS(m,x,y,size,stringBuffer);
}

void textMeshPrintf(textMesh *m, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsprintf(stringBuffer,format,ap);
	va_end(ap);
	if(stringBuffer[0]==0){return;}

	textMeshAddString(m,stringBuffer);
}

void textMeshDigit(textMesh *m, int digit,int x,int y){
	const int   glyphWidth = 16;
	const float glyphSize  = 1.f / 64.f;
	const float gx = 4.f/32.f + (((float)(digit&0x07))*glyphSize);
	const float gy = (28.f/32.f-glyphSize) - ((digit>>3)*glyphSize);

	textMeshAddVert( m, x           , y           , (gx          )*128.f, (gy          )*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, (gx+glyphSize)*128.f, (gy+glyphSize)*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x+glyphWidth, y           , (gx+glyphSize)*128.f, (gy          )*128.f,0xFFFFFFFF);

	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, (gx+glyphSize)*128.f, (gy+glyphSize)*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x           , y           , (gx          )*128.f, (gy          )*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x           , y+glyphWidth, (gx          )*128.f, (gy+glyphSize)*128.f,0xFFFFFFFF);
}

void textMeshNumber(textMesh *m, int number){
	int x = m->sx;
	int y = m->sy;

	if(number == 0){
		textMeshDigit(m,0,x,y);
		return;
	}
	while(number != 0){
		textMeshDigit(m,number % 10,x,y);
		number = number / 10;
		x-= 12;
	}
}

void textMeshBox(textMesh *m, int x, int y, int w, int h, float u, float v, float uw, float vh, uint32_t rgba){
	textMeshAddVert( m, x  ,y+h,(u   )*128.f,(v+vh)*128.f,rgba);
	textMeshAddVert( m, x+w,y  ,(u+uw)*128.f,(v   )*128.f,rgba);
	textMeshAddVert( m, x  ,y  ,(u   )*128.f,(v   )*128.f,rgba);
	textMeshAddVert( m, x+w,y  ,(u+uw)*128.f,(v   )*128.f,rgba);
	textMeshAddVert( m, x  ,y+h,(u   )*128.f,(v+vh)*128.f,rgba);
	textMeshAddVert( m, x+w,y+h,(u+uw)*128.f,(v+vh)*128.f,rgba);
}
