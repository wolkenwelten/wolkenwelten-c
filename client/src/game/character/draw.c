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
#include "draw.h"
#include "../blockType.h"
#include "character.h"
#include "../light.h"
#include "../../gfx/gfx.h"
#include "../../gfx/mat.h"
#include "../../gfx/mesh.h"
#include "../../gfx/shader.h"
#include "../../gfx/shadow.h"
#include "../../misc/options.h"
#include "../../sdl/sdl.h"
#include "../../tmp/objs.h"
#include "../../../../common/src/misc/misc.h"

#include <math.h>

void characterUpdateAnimation(character *c){
	c->animationTicksLeft -= msPerTick;
	c->breathing          += msPerTick;
	if(c->animationTicksLeft <= 0){
		c->animationTicksLeft = 0;
		c->animationTicksMax  = 0;
		c->animationIndex     = 0;
	}
	if(c->flags & CHAR_GLIDE){
		c->gliderFade += 0.02f;
	}else{
		c->gliderFade -= 0.02f;
	}
	c->gliderFade = MINMAX(0.f,1.f,c->gliderFade);
}

static void characterShadesDraw(const character *c){
	float sneakOff = 0.f;
	if(c->flags & CHAR_SNEAK){sneakOff = 1.f;}
	const float breath = sinf((float)(c->breathing-256)/512.f)*6.f;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff+breath/128.f,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw,-(c->rot.pitch+sneakOff*30.f)/3.f + breath);
	matMulTrans(matMVP,0.f,0.1f,-0.2f);
	matMulScale(matMVP,0.5f, 0.5f, 0.5f);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshSunglasses);
}

static void characterGliderDraw(const character *c){
	static u64 ticks = 0;
	if(c->gliderFade < 0.01f){return;}
	const float breath = sinf((float)(c->breathing-384)/512.f)*4.f;
	float deg  = ((float)++ticks*0.4f);
	float yoff = cosf(deg*2.1f) * player->shake;
	float xoff = sinf(deg*1.3f) * player->shake;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw+xoff,-(c->rot.pitch-((1.f - c->gliderFade)*90.f)-breath+yoff));
	matMulTrans(matMVP,0.f,0.4f,-0.2f);
	matMulScale(matMVP,c->gliderFade, c->gliderFade, c->gliderFade);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshGlider);
}

void characterCalcMVP(const character *c, float out[16]){
	const float breath = sinf((float)c->breathing/512.f) * 6.f;
	matMov(out, matView);
	matMulTrans(out, c->pos.x, c->pos.y+c->yoff+breath/128.f, c->pos.z);
	matMulRotYX(out, -c->rot.yaw, -c->rot.pitch/6.f + breath);
	matMul(out, out,matProjection);
}

void characterDraw(const character *c){
	if(c == NULL)       {return;}
	if(c->eMesh == NULL){return;}
	if((c == player) && (!optionThirdPerson)){return;}
	shadowAdd(c->pos,0.75f);

	const float brightness = lightAtPos(c->pos);
	if(c->effectValue){
		const float effectMult = 1.f - (c->effectValue / 31.f);
		const float lowBrightness = brightness * effectMult * effectMult;
		shaderColor(sMesh, brightness, lowBrightness, lowBrightness, 1.f);
	}else{
		shaderColorSimple(sMesh, brightness);
	}
	characterCalcMVP(c, matMVP);
	shaderMatrix(sMesh,matMVP);
	meshDraw(c->eMesh);
	shaderColorSimple(sMesh, brightness);

	characterShadesDraw(c);
	characterGliderDraw(c);
}

void characterDrawAll(){
	shaderBind(sMesh);
	for(uint i=0;i<characterCount;i++){
		if(characterList[i].nextFree != NULL){ continue; }
		characterDraw(&characterList[i]);
	}
}

void characterDrawConsHighlight(const character *c){
	static uint counter = 0;
	if((c->flags & CHAR_CONS_MODE) == 0){return;}
	vec los = characterLOSBlock(c,true);
	if(los.x < 0){return;}
	const float a = 0.7f + cosf((++counter&0x7F)/128.f*PI*2)*0.15f;
	blockTypeDraw(3, vecNew(los.x+0.5f,los.y+0.5f,los.z+0.5f),a,0);
}

static void characterDrawGliderFirstPerson(const character *c){
	if(optionThirdPerson || (c == NULL)){return;}
	static u64 ticks = 0;
	float matViewAI[16];
	if(c->gliderFade < 0.01f){return;}

	const float shake = MINMAX(0.f, 0.2f, c->shake);
	float deg  = ((float)++ticks*0.4f);
	float yoff = fabsf(cosf(deg * 2.1f) * shake);
	float xoff = fabsf(sinf(deg * 1.3f) * shake);

	float breath = sinf((float)(c->breathing-256)/512.f)*3.f;

	matTranslation(matViewAI, 0.f, c->yoff * 0.35f + 0.9f, -0.65f);
	matMulRotYX(matViewAI, 0.f+xoff, c->rot. pitch * -0.08f + yoff + breath * 0.01f);
	matMulScale(matViewAI, c->gliderFade, c->gliderFade, c->gliderFade);
	matMul(matViewAI, matViewAI, matProjection);
	shaderMatrix(sMesh, matViewAI);
	meshDraw(meshGlider);
}

void characterDrawFirstPerson(const character *c){
	const float brightness = lightAtPos(c->pos);
	shaderBind(sMesh);
	shaderColorSimple(sMesh, brightness);

	characterDrawGliderFirstPerson(c);
}
