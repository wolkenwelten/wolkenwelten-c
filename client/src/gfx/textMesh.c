#include "textMesh.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gfx/shader.h"
#include "../tmp/assets.h"

#include "../../../common/src/mods/mods.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../gfx/gl.h"

char stringBuffer[8192];

char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

u32 colorPalette[16] = {
	0x00000000,
	0xFF0E117F,
	0xFF299413,
	0xFF0C9789,
	0xFF9B490D,
	0xFF841399,
	0xFF9B8815,
	0xFFA89DAF,

	0xFF798A8F,
	0xFF1F41F2,
	0xFF29F013,
	0xFF19E7D9,
	0xFFFB591D,
	0xFFC423E9,
	0xFFFBE82A,
	0xFFF8FFF0
};

textMesh *textMeshNew(uint bufferSize){
	textMesh *m = malloc(sizeof(textMesh));
	if(m == NULL){
		fprintf(stderr,"Error allocation textMesh\n");
		return NULL;
	}
	m->vbo        = m->dataCount =  0;
	m->sx         = m->sy        =  0;
	m->mx         = m->my        = -1;
	m->size       = 1;
	m->tex        = tGui;
	m->finished   = 0;
	m->usage      = GL_STREAM_DRAW;
	m->fgc        = colorPalette[15];
	m->bgc        = colorPalette[ 0];
	m->dataBuffer = malloc(sizeof(vertex2D) * bufferSize);
	m->bufferSize = bufferSize;

	return m;
}

void textMeshFree(textMesh *m){
	if(m == NULL){return;}
	if(m->vbo){
		glDeleteBuffers(1,&m->vbo);
	}
	if(m->dataBuffer != NULL){
		free(m->dataBuffer);
		m->dataBuffer = NULL;
	}
}

void textMeshEmpty(textMesh *m){
	m->dataCount =  0;
	m->sx        =  0;
	m->sy        =  0;
	m->mx        = -1;
	m->my        = -1;
	m->size      =  1;
	m->finished  =  0;
}

void textMeshAddVert(textMesh *m, i16 x, i16 y, i16 u, i16 v, u32 rgba){
	if(m->dataCount >= m->bufferSize){
		fprintf(stderr,"textMesh dataBuffer OVERFLOW\n");
		return;
	}
	m->dataBuffer[m->dataCount++] = (vertex2D){x,y,u,v,rgba};
}

void textMeshDraw(textMesh *m){
	if(!m->vbo) {
		glGenBuffers(1,&m->vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	if(!m->finished){
		glBufferData(GL_ARRAY_BUFFER, m->dataCount*sizeof(vertex2D), m->dataBuffer, m->usage);
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

static int textMeshParseEscapeCode(textMesh *m, const char *str){
	int off = 0;
	for(int i=0;i<7;i++){
		if(str[i] == 0){return i;}
	}
	if(str[1] != '['){return 1;}
	if(str[3] != ';'){return 1;}
	if(str[6] != 'm'){return 1;}
	if(str[2] == '1'){
		off = 8;
	}else if(str[2] != '0'){
		return 1;
	}
	if(str[4] != '3'){return 1;}
	if((str[5] < '0') || (str[5] > '8')){return 1;}
	m->fgc = colorPalette[off + str[5] - '0'];
	return 7;
}

void textMeshAddGlyph(textMesh *m, int x, int y, int size, u8 c, u32 fgc, u32 bgc){
	float gx;
	float gy;
	int   glyphWidth = 8*size;
	float glyphSize  = 1.f / 64.f;

	if(x < -size){return;}
	if(y < -size){return;}
	if(c==0)     {return;}
	if(c==' ')   {return;}

	if(size == 1){
		glyphSize = 1.f / 128.f;
	}

	gx = ((float)(c&0x0f))*glyphSize + (m->font * 1.f/4.f);
	gy = ((float)(63-((c>>4)&0x0f)))*glyphSize;
	if(size == 1){
		gy = ((float)(15-((c>>4)&0x0f)))*glyphSize + 24.f/32.f;
	}

	textMeshAddVert( m, x           , y           , ((gx          )*128.f), ((gy          )*128.f),fgc);
	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, ((gx+glyphSize)*128.f), ((gy+glyphSize)*128.f),fgc);
	textMeshAddVert( m, x+glyphWidth, y           , ((gx+glyphSize)*128.f), ((gy          )*128.f),fgc);

	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, ((gx+glyphSize)*128.f), ((gy+glyphSize)*128.f),fgc);
	textMeshAddVert( m, x           , y           , ((gx          )*128.f), ((gy          )*128.f),fgc);
	textMeshAddVert( m, x           , y+glyphWidth, ((gx          )*128.f), ((gy+glyphSize)*128.f),fgc);

	if(bgc & 0xFF000000){
		textMeshBox(m,x,y,glyphWidth,glyphWidth,19.f/32.f, 31.f/32.f,1.f/32.f,1.f/32.f,bgc);
	}
}

void textMeshAddStrPS(textMesh *m, int x, int y, int size, const char *str){
	const int glyphWidth = 8*size;
	const int lineHeight = 10*size;
	int maxX = m->mx;
	if(maxX == -1){maxX = screenWidth;}
	int maxY = m->my;
	if(maxY == -1){maxY = screenHeight;}

	if(str == NULL){return;}
	while(*str != 0){
		if(y > maxY){
			return;
		}
		if(x > maxX){
			if(m->wrap == 0){return;}
			x = m->sx;
			y += lineHeight;
			if(x+glyphWidth > screenWidth){
				return;
			}else{
				continue;
			}
		}
		if(*str == '\n'){
			x = m->sx;
			m->sy += lineHeight;
			y += lineHeight;
			str++;
			continue;
		}else if(*str == '\r'){
			str++;
			continue;
		}else if(*str == '\t'){
			str++;
			x = (((x - m->sx) / (glyphWidth*4) ) + 1 ) * (glyphWidth*4);
			continue;
		}else if(*str == '\033'){
			str += textMeshParseEscapeCode(m,str);
			continue;
		}

		textMeshAddGlyph(m,x,y,size,*str,m->fgc,m->bgc);
		x += glyphWidth;
		str++;
	}
}

void textMeshAddString(textMesh *m, const char *str){
	return textMeshAddStrPS(m,m->sx,m->sy,m->size,str);
}

void textMeshPrintfPS(textMesh *m, int x, int y, int size, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(stringBuffer,sizeof(stringBuffer),format,ap);
	va_end(ap);
	stringBuffer[sizeof(stringBuffer)-1]=0;
	if(stringBuffer[0]==0){return;}

	m->sx = x;
	m->sy = y;
	textMeshAddStrPS(m,x,y,size,stringBuffer);
}

void textMeshPrintf(textMesh *m, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(stringBuffer,sizeof(stringBuffer),format,ap);
	va_end(ap);
	stringBuffer[sizeof(stringBuffer)-1]=0;
	if(stringBuffer[0]==0){return;}

	textMeshAddString(m,stringBuffer);
}

void textMeshPrintfRA(textMesh *m, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(stringBuffer,sizeof(stringBuffer),format,ap);
	va_end(ap);
	stringBuffer[sizeof(stringBuffer)-1]=0;
	if(stringBuffer[0]==0){return;}
	m->sx -= strnlen(stringBuffer,sizeof(stringBuffer))*(m->size*8);

	textMeshAddString(m,stringBuffer);
}


void textMeshDigit(textMesh *m, int x, int y, int size, int digit){
	const int   glyphWidth = 16*size;
	const float glyphSize  = 1.f / 64.f;
	const float gx = 4.f/32.f + (((float)(digit&0x07))*glyphSize) + (m->font * 1.f/4.f);
	const float gy = (28.f/32.f-glyphSize) - ((digit>>3)*glyphSize);

	textMeshAddVert( m, x           , y           , (gx          )*128.f, (gy          )*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, (gx+glyphSize)*128.f, (gy+glyphSize)*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x+glyphWidth, y           , (gx+glyphSize)*128.f, (gy          )*128.f,0xFFFFFFFF);

	textMeshAddVert( m, x+glyphWidth, y+glyphWidth, (gx+glyphSize)*128.f, (gy+glyphSize)*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x           , y           , (gx          )*128.f, (gy          )*128.f,0xFFFFFFFF);
	textMeshAddVert( m, x           , y+glyphWidth, (gx          )*128.f, (gy+glyphSize)*128.f,0xFFFFFFFF);
}

void textMeshNumber(textMesh *m, int x, int y, int size, int number){
	if(number == 0){
		textMeshDigit(m,x,y,size,0);
		return;
	}
	while(number != 0){
		textMeshDigit(m,x,y,size,number % 10);
		number = number / 10;
		x-= 12*size;
	}
}

void textMeshBox(textMesh *m, int x, int y, int w, int h, float u, float v, float uw, float vh, u32 rgba){
	textMeshAddVert( m, x  ,y+h,(u   )*128.f,(v+vh)*128.f,rgba);
	textMeshAddVert( m, x+w,y  ,(u+uw)*128.f,(v   )*128.f,rgba);
	textMeshAddVert( m, x  ,y  ,(u   )*128.f,(v   )*128.f,rgba);
	textMeshAddVert( m, x+w,y  ,(u+uw)*128.f,(v   )*128.f,rgba);
	textMeshAddVert( m, x  ,y+h,(u   )*128.f,(v+vh)*128.f,rgba);
	textMeshAddVert( m, x+w,y+h,(u+uw)*128.f,(v+vh)*128.f,rgba);
}

void textMeshSolidBox(textMesh *m, int x, int y, int w, int h, u32 rgba){
	textMeshBox(m,x, y,w, h,19.f/32.f, 31.f/32.f,1.f/32.f,1.f/32.f,rgba);
}

void textMeshItemSprite(textMesh *m, int x, int y, int size, int itemID){
	const float ITEMTILE = (1.f/32.f);
	int u = itemID % 32;
	int v = itemID / 32;
	textMeshBox(m,x,y,size,size,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);
}

void textMeshSlot(textMesh *m, int x, int y, int size, int style){
	float u = 20.f;
	if(style == 1)     { u = 21.f; }
	else if(style == 2){ u = 22.f; }
	else if(style == 3){return;    }
	textMeshBox(m,x,y,size,size,u/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
}

void textMeshItemSlot(textMesh *m, int x, int y, int size, int style, int itemID, int amount){
	textMeshSlot(m,x,y,size,style);
	if(itemID <= 0){return;}
	const int itemsize    = size - size / 6;
	const int itemsizeoff = (size-itemsize)/2;
	textMeshItemSprite(m,x+itemsizeoff,y+itemsizeoff,itemsize,itemID);
	textMeshNumber(m,x+size-size/4,y+(size-size/4-size/32),1,amount);
}

void textMeshItem(textMesh *m, int x, int y, int size, int style, item *itm){
	textMeshSlot(m,x,y,size,style);
	if(itemIsEmpty(itm)){return;}
	const int itemsize    = size - size / 6;
	const int itemsizeoff = (size-itemsize)/2;
	textMeshItemSprite(m,x+itemsizeoff,y+itemsizeoff,itemsize,itm->ID);
	if(getStackSizeDispatch(itm) <= 1){
		if(hasGetMagSize(itm)){
			textMeshNumber(m,x+size/4,y+size/8,1,itemGetAmmo(itm));
			textMeshNumber(m,x+size-size/4,y+size/8,1,getMagSizeDispatch(itm));
			textMeshDigit (m,x+size/2-size/16,y+size/8, 1, 10);
		}
	}else{
		textMeshNumber(m,x+size-size/4,y+(size-size/4-size/32),1,itm->amount);
	}
}

void textMeshVGradient(textMesh *m, int x, int y, int w, int h, u32 c1, u32 c2){
	float u = 19.f/32.f*128.f;
	float v = 31.f/32.f*128.f;
	float s =  1.f/32.f*128.f;

	textMeshAddVert(m,x  ,y  ,u  ,v  ,c1);
	textMeshAddVert(m,x  ,y+h,u  ,v+s,c2);
	textMeshAddVert(m,x+w,y+h,u+s,v+s,c2);

	textMeshAddVert(m,x+w,y+h,u+s,v+s,c2);
	textMeshAddVert(m,x+w,y  ,u+s,v  ,c1);
	textMeshAddVert(m,x  ,y  ,u  ,v  ,c1);
}
