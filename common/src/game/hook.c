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

#include "../game/hook.h"

#include "../game/being.h"
#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../mods/api_v1.h"
#include "../misc/misc.h"
#include "../../../common/src/game/rope.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

hook hookList[128];
uint hookCount = 0;

hook *hookNew(character *p){
	hook *ghk = NULL;
	for(uint i=0;i<hookCount;i++){
		if(hookList[i].parent == NULL){
			ghk = &hookList[i];
			break;
		}
	}
	if(ghk == NULL){
		ghk = &hookList[hookCount++];
	}
	u32 flags = ROPE_DIRTY;
	if(characterGetMaxHookLen(p) > 128.f){flags |= ROPE_TEX_CHAIN;}

	ghk->parent       = p;
	ghk->ent          = entityNew(vecAdd(vecNew(0,1,0),p->pos),vecNew(-p->rot.yaw,-p->rot.pitch-90.f,p->rot.roll));
	ghk->ent->vel     = vecAdd(vecMulS(vecDegToVec(p->rot),1.3f),p->vel);
	ghk->ent->eMesh   = meshHook;
	ghk->ent->flags   = ENTITY_NOREPULSION;
	ghk->rope         = ropeNew(hookGetBeing(ghk),characterGetBeing(p),flags);
	ghk->rope->length = -1.f;
	ghk->attached     = 0;

	ghk->hooked       = false;
	ghk->returning    = false;
	ghk->goalLength   = 0;

	return ghk;
}

void hookFree(hook *ghk){
	entityFree(ghk->ent);
	ghk->ent    = NULL;
	ghk->parent = NULL;
	ropeFree(ghk->rope);
}

float hookGetLength(const hook *ghk){
	return vecMag(vecSub(ghk->ent->pos,ghk->parent->pos));
}

float hookGetRopeLength(const hook *ghk){
	return ropeGetLength(ghk->rope);
}

bool hookReturnToParent(hook *ghk,float speed){
	vec dist = vecSub(ghk->ent->pos,ghk->parent->pos);
	float d  = vecMag(dist);
	speed = d / 8.f;
	if(speed < 1.f){speed = 1.f;}
	if(d <= 2){return true;}
	ghk->ent->vel = vecMulS(vecNorm(dist),-speed);

	if(ghk->ent->flags & ENTITY_COLLIDE){
		ghk->ent->pos = vecAdd(ghk->ent->pos,ghk->ent->vel);
	}
	return false;
}

void hookReturnHook(hook *ghk){
	ghk->returning    = true;
	ghk->hooked       = false;
	ghk->ent->vel     = vecZero();
	ghk->rope->a      = hookGetBeing(ghk);
	ghk->rope->length = -1.f;
	ghk->rope->flags |= ROPE_DIRTY;
	hookReturnToParent(ghk,0.1f);
}

bool hookUpdate(hook *ghk){
	entityUpdate(ghk->ent);

	if(ghk->returning && hookReturnToParent(ghk,0.01f)){
		return true;
	}else if(ghk->hooked && !ghk->returning){
		ghk->ent->vel = vecZero();
		if(!(ghk->ent->flags & ENTITY_COLLIDE) && (ghk->attached == 0)){
			hookReturnHook(ghk);
		}
		if((ghk->attached != 0) && (ghk->rope->a == 0)){
			hookReturnHook(ghk);
		}
	}else{
		if(!ghk->hooked && !ghk->returning && (ghk->ent->flags & ENTITY_COLLIDE)){
			ghk->ent->vel     = vecZero();
			ghk->ent->flags  |= ENTITY_NOCLIP;
			ghk->hooked       = true;
			ghk->attached     = 0;
			ghk->goalLength   = hookGetLength(ghk);
			ghk->rope->length = ghk->goalLength;
			ghk->rope->flags |= ROPE_DIRTY;
			sfxPlay(sfxHookHit,1.f);
			sfxLoop(sfxHookRope,0.f);
			unsigned char b  = worldGetB(ghk->ent->pos.x,ghk->ent->pos.y,ghk->ent->pos.z);
			if(b){ fxBlockBreak(vecFloor(ghk->ent->pos),b,0);}
		}else{
			being closest = beingClosest(ghk->ent->pos,1.f);
			if((closest != 0) && (closest != characterGetBeing(ghk->parent))){
				ghk->ent->vel     = vecZero();
				ghk->ent->flags  |= ENTITY_NOCLIP;
				ghk->hooked       = true;
				ghk->goalLength   = hookGetLength(ghk);
				ghk->rope->length = ghk->goalLength;
				ghk->attached     = closest;
				ghk->rope->a      = closest;
				ghk->ent->pos     = beingGetPos(closest);
				ghk->rope->flags |= ROPE_DIRTY;
				//sfxPlay(sfxHookHit,1.f);
				//sfxPlay(sfxUngh,1.f);
				fxBlockBreak(ghk->ent->pos,2,0);
				sfxLoop(sfxHookRope,0.f);
			}else if(!ghk->hooked && !ghk->returning){
				sfxLoop(sfxHookRope,1.f);
				if(hookGetLength(ghk) > characterGetMaxHookLen(ghk->parent)){
					hookReturnHook(ghk);
				}
			}
		}
	}
	if(ghk->ent->pos.y < -256.f){
		return true;
	}
	return false;
}

bool hookGetHooked(const hook *ghk){
	if(ghk == NULL){return false;}
	return ghk->hooked;
}
float hookGetGoalLength(const hook *ghk){
	if(ghk == NULL){return false;}
	return ghk->goalLength;
}
void hookSetGoalLength(hook *ghk, float len){
	ghk->goalLength = len;
	if(ghk->hooked && !ghk->returning){
		ghk->rope->length = ghk->goalLength;
		ghk->rope->flags |= ROPE_DIRTY;
	}
}

hook *hookGetByBeing(being b){
	const uint i = beingID(b);
	if(beingType(b) != BEING_HOOK){ return NULL; }
	if(i >= hookCount)            { return NULL; }
	return &hookList[i];
}

uint hookGetBeing(const hook *c){
	if(c == NULL){return 0;}
	return beingHook(c - &hookList[0]);
}
