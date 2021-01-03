#include "rain.h"

#include "../game/character.h"
#include "../game/weather.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"

uint rainVAO;
uint rainVBO;

#include <math.h>

void rainInitGfx(){
	glGenVertexArrays(1, &rainVAO);
	glGenBuffers     (1, &rainVBO);
	glBindVertexArray    (rainVAO);
	glEnableVertexAttribArray(0);
}

void rainFakeDrops(){
	if(rainDuration <= 0){return;}
	vec pos = player->pos;
	pos.y = (float)(((int)pos.y) & 0xFF00) + (32.f - 256.f);
	for(uint i=0;i<3;i++){
		float v = 16.f;
		for(int ii=0;ii<6;ii++){
			rainNew(vecAdd(pos,vecMul(vecRng(), vecNew( v,0.f, v))));
			v *= 2.f;
		}
		pos.y += 256.f;
	}
}

void rainDrawAll(){
	if(!rainCount){return;}
	rainFakeDrops();

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

void rainRecvUpdate(const packet *p){
	const vec pos  = vecNew(p->v.f[0],p->v.f[1],p->v.f[2]);
	const vec dist = vecSub(pos,player->pos);
	const float dd = vecDot(dist,dist);
	if(dd > renderDistanceSquare){return;}
	rainNew(pos);
}