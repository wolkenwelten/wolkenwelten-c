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
#include "fbo.h"
#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gui/gui.h"

#include <stdio.h>

uint fboBlitVBO = 0;
uint fboBlitVAO = 0;
bool renderToFBO = false;

framebufferObject fboGame;

void fboFree(framebufferObject *fbo){
	fbo->width = fbo->height = 0;
	if(fbo->ID){glDeleteFramebuffers(1, &fbo->ID);}
	fbo->ID = 0;
	if(fbo->texColor){glDeleteTextures(1, &fbo->texColor);}
	fbo->texColor = 0;
	if(fbo->rboDepth){glDeleteRenderbuffers(1, &fbo->rboDepth);}
	fbo->rboDepth = 0;
}

void fboInit(framebufferObject *fbo, uint w, uint h){
	fboFree(fbo);

	glGenFramebuffers(1, &fbo->ID);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->ID);
	fbo->width = w;
	fbo->height = h;

	glGenTextures(1, &fbo->texColor);
	glBindTexture(GL_TEXTURE_2D, fbo->texColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texColor, 0);

	glGenRenderbuffers(1, &fbo->rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, fbo->rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->rboDepth);
}

void fboBind(framebufferObject *fbo, uint w, uint h){
	if((fbo == 0) || (fbo->width != w) || (fbo->height != h)){
		fboInit(fbo, w, h);
	}else{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->ID);
	}
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		fprintf(stderr,"Error binding FBO, Framebuffer not complete\n");
	}
	glViewport(0, 0, w, h);
	glScissor(0, 0, w, h);
}

void fboBlit(framebufferObject *fbo, uint x, uint y, uint w, uint h){
	vertex2D v[6];
	v[0] = (vertex2D){x  , y+h,   0,   0, 0xFFFF80A0};
	v[1] = (vertex2D){x+w, y  , 128, 128, 0xFFFF80A0};
	v[2] = (vertex2D){x  , y  ,   0, 128, 0xFFFF80A0};
	v[3] = (vertex2D){x+w, y  , 128, 128, 0xFFFF80A0};
	v[4] = (vertex2D){x  , y+h,   0,   0, 0xFFFF80A0};
	v[5] = (vertex2D){x+w, y+h, 128,   0, 0xFFFF80A0};

	gfxGroupStart("FBO");
	glDisable(GL_BLEND);
	shaderBind(sBlit);
	shaderMatrix(sBlit, matOrthoProj);

	if(!fboBlitVAO){
		glGenVertexArrays(1, &fboBlitVAO);
		glBindVertexArray(fboBlitVAO);
		glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
		glEnableVertexAttribArray(SHADER_ATTRIDX_TEX);
	}else{
		glBindVertexArray(fboBlitVAO);
	}

	if(!fboBlitVBO){
		glGenBuffers(1, &fboBlitVBO);
	}
	glBindBuffer(GL_ARRAY_BUFFER, fboBlitVBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vertex2D), v, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(SHADER_ATTRIDX_POS, 2, GL_SHORT , GL_FALSE, sizeof(vertex2D), (void *)(((char *)&v[0].x) - ((char *)v)));
	glVertexAttribPointer(SHADER_ATTRIDX_TEX, 2, GL_SHORT , GL_FALSE, sizeof(vertex2D), (void *)(((char *)&v[0].u) - ((char *)v)));

	textureBindID(fbo->texColor);
	//textureBindID(tBlocks->ID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	vboTrisCount += 2;
	drawCallCount++;

	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh, matOrthoProj);
	glEnable(GL_BLEND);
}

void fboBindScreen(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
	glScissor(0, 0, screenWidth, screenHeight);
}
