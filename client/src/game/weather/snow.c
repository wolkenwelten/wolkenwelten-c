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
#include "snow.h"
#include "../../gfx/gfx.h"
#include "../../gfx/gl.h"
#include "../../gfx/mat.h"
#include "../../gfx/shader.h"
#include "../../game/character.h"

uint snowVAO;
uint snowVBO;
uint snowVBOSize = 0;

#ifdef __x86_64__
int snowFakeIters = 128;
#else
int snowFakeIters = 16;
#endif

void snowInitGfx(){
	glGenVertexArrays(1, &snowVAO);
	glGenBuffers     (1, &snowVBO);
	glBindVertexArray    (snowVAO);
	glEnableVertexAttribArray(SHADER_ATTRIDX_POS);
}

void snowDrawAll(){
	gfxGroupStart("Snow");
	if(!snowCount){return;}

	shaderBind(sRain);
	matMul(matMVP,matView,matProjection);
	shaderColor(sRain,0.85f,0.9f,0.95f,1.f);
	shaderMatrix(sRain,matMVP);
	shaderSizeMul(sRain,player->zoomFactor);
	glDepthMask(GL_FALSE);

	glBindVertexArray(snowVAO);
	glBindBuffer(GL_ARRAY_BUFFER,snowVBO);

	glBufferData(GL_ARRAY_BUFFER, snowCount*sizeof(snowDrop), glSnowDrops, GL_DYNAMIC_DRAW);
	snowVBOSize = snowCount;

	glVertexAttribPointer(SHADER_ATTRIDX_POS, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_POINTS,0,snowCount);

	glDepthMask(GL_TRUE);
	gfxGroupEnd();
}

void snowRecvUpdate(const packet *p){
	const vec pos  = vecNew(p->v.f[0],p->v.f[1],p->v.f[2]);
	const vec dist = vecSub(pos,player->pos);
	const float dd = vecDot(dist,dist);
	if(dd > renderDistanceSquare){return;}
	snowNew(pos);
}
