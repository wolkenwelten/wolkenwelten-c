#include "rain.h"

#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"

uint rainVAO;
uint rainVBO;

void rainInitGfx(){
	glGenVertexArrays(1, &rainVAO);
	glGenBuffers     (1, &rainVBO);
	glBindVertexArray    (rainVAO);
	glEnableVertexAttribArray(0);
}

void rainDrawAll(){
	if(!rainCount){return;}

	shaderBind(sRain);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);
	shaderSizeMul(sCloud,1.f + (player->aimFade * player->zoomFactor));
	glDepthMask(GL_FALSE);

	glBindVertexArray(rainVAO);
	glBindBuffer(GL_ARRAY_BUFFER,rainVBO);
	glBufferData(GL_ARRAY_BUFFER, rainCount*sizeof(glRainDrop), glRainDrops, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_POINTS,0,rainCount);

	glDepthMask(GL_TRUE);
}
