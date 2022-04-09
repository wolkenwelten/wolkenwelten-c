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
#include "../../../../common/src/game/item.h"

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

static void characterActiveItemDraw(const character *c){
	const item *activeItem;
	mesh *aiMesh;
	float sneakOff = 0.f;
	if(c->flags & CHAR_SNEAK){sneakOff = 1.f;}

	activeItem = &c->inventory[c->activeItem];
	if(activeItem == NULL)     {return;}
	if(itemIsEmpty(activeItem)){return;}
	aiMesh = itemGetMesh(activeItem);
	if(aiMesh == NULL)         {return;}

	const float breath = cosf((float)c->breathing/512.f)*4.f;

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw+(15.f*sneakOff),-c->rot.pitch+breath);

	const float ix =  0.4f - (sneakOff/20.f);
	const float iy = -0.2f;
	const float iz = -0.3f;
	float hitOff,y;

	switch(c->animationIndex){
	case animationHit:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.3f);
		y = iy+c->yoff-(hitOff/8);
		matMulTrans(matMVP,ix-hitOff*0.2f,y+(hitOff/3),iz - hitOff*0.5f);
		matMulRotYX(matMVP,hitOff*5.f,hitOff*-20.f);
	break;

	case animationFire:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.5f);
		matMulTrans(matMVP,ix,c->yoff+iy,iz + hitOff*0.3f);
		matMulRotYX(matMVP,hitOff*10.f,hitOff*45.f);
	break;

	case animationReload:
		hitOff = animationInterpolationSustain(c->animationTicksLeft,c->animationTicksMax,0.3f,0.5f);
		y = iy+c->yoff-(hitOff/8);
		matMulTrans(matMVP,ix-hitOff*0.5f,y-(hitOff*0.5f),iz - hitOff*0.2f);
		matMulRotYX(matMVP,hitOff*15.f,hitOff*-55.f);
	break;

	case animationEmpty:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,0.5f);
		matMulTrans(matMVP,ix,c->yoff+iy,iz + hitOff*0.1f);
		matMulRotYX(matMVP,hitOff*3.f,hitOff*9.f);
	break;

	case animationEat:
		hitOff = animationInterpolation(c->animationTicksLeft,c->animationTicksMax,1.f)*3.f;
		if(hitOff < 1.f){
			matMulTrans(matMVP,ix-hitOff*0.4,c->yoff+iy,iz - hitOff*0.2f);
			matMulRotYX(matMVP,hitOff*20.f,hitOff*40.f);
		}else if(hitOff < 2.f){
			hitOff = hitOff-1.f;
			matMulTrans(matMVP,ix-0.4f,c->yoff+iy-hitOff*0.2f,iz - 0.2f);
			matMulRotYX(matMVP,hitOff*60.f+20.f,hitOff*120.f+40.f);
			matMulScale(matMVP, 1.f-hitOff, 1.f-hitOff, 1.f-hitOff);
		}else if(hitOff < 3.f){
			hitOff = 1.f-(hitOff-2.f);
			matMulTrans(matMVP,ix-hitOff*0.4,c->yoff+iy,iz + hitOff*0.4f);
			matMulRotYX(matMVP,hitOff*20.f,hitOff*40.f);
		}
	break;

	case animationSwitch:
		hitOff = (float)c->animationTicksLeft / (float)c->animationTicksMax;
		y = iy+c->yoff-(hitOff/8);
		matMulTrans(matMVP,ix-hitOff*0.5f,y-(hitOff*0.5f),iz - hitOff*0.2f);
		matMulRotYX(matMVP,hitOff*30.f,hitOff*-70.f);
	break;
	};

	matMulScale(matMVP,0.5f, 0.5f, 0.5f);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(aiMesh);
}

void characterDraw(character *c){
	if(c == NULL)       {return;}
	if(c->eMesh == NULL){return;}
	if((c == player) && (!optionThirdPerson)){return;}
	shadowAdd(c->pos,0.75f);

	const float breath = sinf((float)c->breathing/512.f)*6.f;
	const float brightness = lightAtPos(c->pos);
	if(c->effectValue){
		const float effectMult = 1.f - (--c->effectValue / 31.f);
		const float lowBrightness = brightness * effectMult * effectMult;
		shaderColor(sMesh, brightness, lowBrightness, lowBrightness, 1.f);
	}else{
		shaderColorSimple(sMesh, brightness);
	}
	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff+breath/128.f,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw,-c->rot.pitch/6.f + breath);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(c->eMesh);
	c->screenPos = matMulVec(matMVP,vecNew(0,0.5f,0));
	shaderColorSimple(sMesh, brightness);

	characterActiveItemDraw(c);
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
	item *activeItem = &player->inventory[player->activeItem];
	if((activeItem == NULL) || itemIsEmpty(activeItem) || (!(player->flags & CHAR_CONS_MODE))){return;}
	const u16 id = activeItem->ID;
	if(id < 256){
		vec los = characterLOSBlock(c,true);
		if(los.x < 0){return;}
		const float a = 0.7f + cosf((++counter&0x7F)/128.f*PI*2)*0.15f;
		blockTypeDraw(id, vecNew(los.x+0.5f,los.y+0.5f,los.z+0.5f),a,0);
	}else{
		vec los = characterLOSBlock(c,false);
		if(los.x < 0){return;}
		const float a = 0.5f + cosf((++counter&0x7F)/128.f*PI*2)*0.15f;
		blockTypeDraw(I_Marble_Block, vecNew(los.x+0.5f,los.y+0.5f,los.z+0.5f),a,-4);
	}
}
