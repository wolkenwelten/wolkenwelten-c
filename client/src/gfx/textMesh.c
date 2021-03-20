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

#include "textMesh.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gfx/shader.h"
#include "../tmp/assets.h"

#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/nujel/nujel.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../gfx/gl.h"

char stringBuffer[8192];

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
	m->vbo        = m->vao       =  0;
	m->sx         = m->sy        =  0;
	m->mx         = m->my        = -1;
	m->size       = 1;
	m->tex        = tGui;
	m->dataCount  = m->finished   = 0;
	m->usage      = GL_DYNAMIC_DRAW;
	m->fgc        = colorPalette[15];
	m->bgc        = colorPalette[ 0];
	m->dataBuffer = malloc(sizeof(vertex2D) * bufferSize);
	m->bufferSize = bufferSize;
	m->vboSize    = 0;

	return m;
}

void textMeshFree(textMesh *m){
	if(m == NULL){return;}
	if(m->vbo){
		glDeleteBuffers(1,&m->vbo);
		m->vboSize = 0;
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
	if(!m->vao) {
		glGenVertexArrays(1, &m->vao);
		glBindVertexArray(m->vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}else{
		glBindVertexArray(m->vao);
	}
	if(!m->vbo) {
		glGenBuffers(1,&m->vbo);
	}
	if(!m->finished){
		glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
		if(gfxUseSubData && (m->vboSize >= m->dataCount)){
			glBufferSubData(GL_ARRAY_BUFFER, 0, m->dataCount*sizeof(vertex2D), m->dataBuffer);
		}else{
			glBufferData(GL_ARRAY_BUFFER, m->dataCount*sizeof(vertex2D), m->dataBuffer, GL_DYNAMIC_DRAW);
			m->vboSize = m->dataCount;
		}
		glVertexAttribPointer(0, 2, GL_SHORT        , GL_FALSE, sizeof(vertex2D), (void *)(((char *)&m->dataBuffer[0].x)    - ((char *)m->dataBuffer)));
		glVertexAttribPointer(1, 2, GL_SHORT        , GL_FALSE, sizeof(vertex2D), (void *)(((char *)&m->dataBuffer[0].u)    - ((char *)m->dataBuffer)));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex2D), (void *)(((char *)&m->dataBuffer[0].rgba) - ((char *)m->dataBuffer)));
		m->finished = 1;
	}
	textureBind(m->tex);
	glDrawArrays(GL_TRIANGLES,0,m->dataCount);

	vboTrisCount += m->dataCount/3;
	drawCallCount++;
}

void textMeshAddGlyph(textMesh *m, int x, int y, int size, u8 c, u32 fgc, u32 bgc){
	float gx;
	float gy;
	int   glyphWidth = 8*size;
	float glyphSize  = 1.f / 64.f;

	if(x < -size){return;}
	if(y < -size){return;}
	if(c==0)     {return;}

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

void textMeshAddGlyphHG(textMesh *m, int x, int y, int size, u8 c, u32 fgc, u32 bgc1, u32 bgc2){
	float gx;
	float gy;
	int   glyphWidth = 8*size;
	float glyphSize  = 1.f / 64.f;

	if(x < -size){return;}
	if(y < -size){return;}
	if(c==0)     {return;}

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

	textMeshHGradient(m,x,y,glyphWidth,glyphWidth,bgc1,bgc2);
}

bool textMeshAddStrPS(textMesh *m, int x, int y, int size, const char *str){
	const int glyphWidth = 8*size;
	const int lineHeight = 10*size;
	int maxX = m->mx;
	if(maxX == -1){maxX = screenWidth;}
	int maxY = m->my;
	if(maxY == -1){maxY = screenHeight;}

	if(str == NULL){return 0;}
	while(*str != 0){
		if(y > maxY){
			return 1;
		}
		if(x > maxX){
			if(m->wrap == 0){return 1;}
			x = m->sx;
			y += lineHeight;
			if(x+glyphWidth > screenWidth){
				return 1;
			}else{
				continue;
			}
		}
		if(((u8)str[0] == 0xCE) && ((u8)str[1] == 0xBB)){ // UTF-8 Lambda
			textMeshAddGlyph(m,x,y,size,20,m->fgc,m->bgc);
			x += glyphWidth;
			str+=2;
		}else if(*str == '\n'){
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
			int fgc = -1, bgc = -1;;
			str += parseAnsiCode(str,&fgc,&bgc);
			if(fgc >= 0){m->fgc = colorPalette[fgc];}
			if(bgc >= 0){m->bgc = colorPalette[bgc];}
			continue;
		}

		textMeshAddGlyph(m,x,y,size,*str,m->fgc,m->bgc);
		x += glyphWidth;
		str++;
	}
	return 0;
}

bool textMeshAddString(textMesh *m, const char *str){
	return textMeshAddStrPS(m,m->sx,m->sy,m->size,str);
}

bool textMeshPrintfPS(textMesh *m, int x, int y, int size, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(stringBuffer,sizeof(stringBuffer),format,ap);
	va_end(ap);
	stringBuffer[sizeof(stringBuffer)-1]=0;
	if(stringBuffer[0]==0){return 0;}

	m->sx = x;
	m->sy = y;
	return textMeshAddStrPS(m,x,y,size,stringBuffer);
}

bool textMeshPrintf(textMesh *m, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(stringBuffer,sizeof(stringBuffer),format,ap);
	va_end(ap);
	stringBuffer[sizeof(stringBuffer)-1]=0;
	if(stringBuffer[0]==0){return 0;}

	return textMeshAddString(m,stringBuffer);
}

bool textMeshPrintfAlignCenter(textMesh *m, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(stringBuffer,sizeof(stringBuffer),format,ap);
	va_end(ap);
	stringBuffer[sizeof(stringBuffer)-1]=0;
	if(stringBuffer[0]==0){return 0;}
	m->sx -= (strnlen(stringBuffer,sizeof(stringBuffer))*(m->size*8))/2;

	return textMeshAddString(m,stringBuffer);
}

bool textMeshPrintfAlignRight(textMesh *m, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vsnprintf(stringBuffer,sizeof(stringBuffer),format,ap);
	va_end(ap);
	stringBuffer[sizeof(stringBuffer)-1]=0;
	if(stringBuffer[0]==0){return 0;}
	m->sx -= strnlen(stringBuffer,sizeof(stringBuffer))*(m->size*8);

	return textMeshAddString(m,stringBuffer);
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
	if(itemGetStackSize(itm) <= 1){
		const int magSize = itemGetMagazineSize(itm);
		if(magSize){
			textMeshNumber(m,x+size/4,y+size/8,1,itemGetAmmo(itm));
			textMeshNumber(m,x+size-size/4,y+size/8,1,magSize);
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

void textMeshHGradient(textMesh *m, int x, int y, int w, int h, u32 c1, u32 c2){
	float u = 19.f/32.f*128.f;
	float v = 31.f/32.f*128.f;
	float s =  1.f/32.f*128.f;

	textMeshAddVert(m,x  ,y  ,u  ,v  ,c1);
	textMeshAddVert(m,x  ,y+h,u  ,v+s,c1);
	textMeshAddVert(m,x+w,y+h,u+s,v+s,c2);

	textMeshAddVert(m,x+w,y+h,u+s,v+s,c2);
	textMeshAddVert(m,x+w,y  ,u+s,v  ,c2);
	textMeshAddVert(m,x  ,y  ,u  ,v  ,c1);
}
