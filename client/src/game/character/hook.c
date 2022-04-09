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
#include "hook.h"
#include "character.h"
#include "../../main.h"
#include "../../sfx/sfx.h"
#include "../../voxel/bigchungus.h"
#include "../../../../common/src/game/hook.h"

void characterFreeHook(character *c){
	if(c->hook != NULL){
		hookFree(c->hook);
		c->hook = NULL;
	}
}

void characterUpdateHook(character *c){
	if(c->hook == NULL){ return; }
	if(hookUpdate(c->hook)){
		hookFree(c->hook);
		c->hook = NULL;
		if(c == player){
			sfxPlay(sfxHookReturned,1.f);
		}
		return;
	}

	if(hookGetHooked(c->hook)){
		const float gl     = hookGetGoalLength(c->hook);
		const float wspeed = characterGetHookWinchS(c);
		const float maxl   = characterGetMaxHookLen(c);
		if(c->controls.y > 0){
			if(gl > 2.f){
				hookSetGoalLength(c->hook,gl-wspeed);
				c->flags |= CHAR_JUMPING;
			}else{
				hookReturnHook(c->hook);
				c->vel = vecMul(c->vel,vecNew(0.5f,1.f,0.5));
				c->vel.y += 0.05f;
				c->flags |= CHAR_JUMPING;
			}
		}
		if((c->flags & CHAR_SNEAK) && (gl < maxl)){
			hookSetGoalLength(c->hook,gl+wspeed);
		}
	}
}

void characterAddHookLength(character *c, float d){
	if((c == NULL) || (c->hook == NULL)){return;}
	const float gl     = hookGetGoalLength(c->hook);
	const float wspeed = characterGetHookWinchS(c);
	const float maxl   = characterGetMaxHookLen(c);
	const float ngl    = MAX(1.f,MIN(maxl,gl+wspeed*d));
	hookSetGoalLength(c->hook,ngl);
}

float characterGetHookLength(const character *c){
	if((c == NULL) || (c->hook == NULL)){return 0.f;}
	return hookGetGoalLength(c->hook);
}

float characterGetRopeLength(const character *c){
	if((c == NULL) || (c->hook == NULL)){return 0.f;}
	return hookGetRopeLength(c->hook);
}

void characterFireHook(character *c){
	if(c->actionTimeout < 0){return;}
	if(!playerChunkActive)  {return;}
	characterCloseGlider(c);
	c->flags |= CHAR_JUMPING;
	characterSetCooldown(c,240);
	if(c->hook == NULL){
		c->hook = hookNew(c);
		sfxPlay(sfxHookFire,1.f);
		characterStartAnimation(c,animationFire,350);
	}else{
		hookReturnHook(c->hook);
		characterStartAnimation(c,animationFire,350);
	}
}

float characterCanHookHit(const character *c){
	const float maxLen = characterGetMaxHookLen(c);
	vec pos = vecAdd(vecNew(0,1,0),c->pos);
	vec vel = vecAdd(vecMulS(vecDegToVec(c->rot),1.3f),c->vel);
	for(int i=0;i<1024;i++){
		pos = vecAdd(pos,vel);
		const vec dis = vecSub(pos,c->pos);
		const float d = vecMag(dis);
		if(d > maxLen){return -1.f;}
		if(checkCollision(pos.x,pos.y,pos.z)){return d;}
		vel.y -= 0.0005f;
	}
	return -1.f;
}
