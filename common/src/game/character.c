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

#include "character.h"
#include "../game/being.h"
#include "../misc/sfx.h"
#include "../network/messages.h"
#include "../network/network.h"
#include "../world/world.h"

#include <stdio.h>
#include <stddef.h>
#include <math.h>

character  characterList[64];
uint       characterCount = 0;

character *characterNew(){
	character *c = NULL;

	for(uint i=0;i<characterCount;i++){
		if(characterList[i].eMesh == NULL){
			c = &characterList[i];
			break;
		}
	}
	if(c == NULL){
		if(characterCount >= countof(characterList)){
			fprintf(stderr,"characterList Overflow!\n");
			return NULL;
		}
		c = &characterList[characterCount++];
	}
	characterInit(c);

	return c;
}

void characterFree(character *c){
	c->eMesh = NULL;
	c->hp = -1;
}

int characterGetHP(const character *c){
	return c->hp;
}

int characterGetMaxHP(const character *c){
	return c->maxhp;
}

void characterToggleGlider(character *c){
	if(c == NULL){return;}
	c->flags ^= CHAR_GLIDE;
}
void characterOpenGlider(character *c){
	if(c == NULL){return;}
	c->flags |= CHAR_GLIDE;
}
void characterCloseGlider(character *c){
	if(c == NULL){return;}
	c->flags &= ~CHAR_GLIDE;
}

void characterOpenConsMode   (      character *c){
	if(c->flags & (CHAR_AIMING | CHAR_THROW_AIM)){characterStopAim(c);}
	if(c->flags & (CHAR_GLIDE)){characterCloseGlider(c);}
	c->flags |= CHAR_CONS_MODE;
}

void characterCloseConsMode  (      character *c){
	if(c->flags & (CHAR_AIMING | CHAR_THROW_AIM)){characterStopAim(c);}
	if(c->flags & (CHAR_GLIDE)){characterCloseGlider(c);}
	c->flags &= ~CHAR_CONS_MODE;
}

void characterToggleConsMode (      character *c){
	if(c->flags & CHAR_CONS_MODE){
		characterCloseConsMode(c);
	}else{
		characterOpenConsMode(c);
	}
}

bool characterIsAiming(const character *c){
	if(c == NULL){return false;}
	return c->flags & CHAR_AIMING;
}
bool characterIsThrowAiming(const character *c){
	if(c == NULL){return false;}
	const uint mask = CHAR_AIMING | CHAR_THROW_AIM;
	return (c->flags & mask) == mask;
}
void characterToggleThrowAim(character *c, float zoom){
	if(c == NULL){return;}
	if(c->flags & CHAR_AIMING){
		characterStopAim(c);
		return;
	}
	c->goalZoomFactor = zoom;
	c->flags |= CHAR_AIMING;
	c->flags |= CHAR_THROW_AIM;
}
void characterToggleAim(character *c, float zoom){
	if(c == NULL){return;}
	c->flags ^= CHAR_AIMING;
	if(c->flags & CHAR_AIMING){
		c->goalZoomFactor = zoom;
	}else{
		c->goalZoomFactor = 1.f;
	}
}
void characterStopAim(character *c){
	if(c == NULL){return;}
	c->goalZoomFactor = 1.f;
	c->flags &= ~(CHAR_AIMING | CHAR_THROW_AIM);
}

void characterSetCooldown(character *c, int cooldown){
	c->actionTimeout = -cooldown;
}

void characterSetPos(character *c, const vec pos){
	c->pos = pos;
}

void characterSetRot(character *c, const vec rot){
	c->rot = rot;
}

void characterSetVelocity(character *c, const vec vel){
	c->vel = vel;
}

void characterSetInaccuracy(character *c, float inacc){
	c->inaccuracy = inacc;
}

void characterAddInaccuracy(character *c, float inc){
	c->inaccuracy += inc;
}

bool characterPlaceBlock(character *c, blockId b){
	if(c->actionTimeout < 0)              { return false; }
	vec los = characterLOSBlock(c,true);
	if(los.x < 0)                         { return false; }
	characterStartAnimation(c,animationHit,240);
	characterSetCooldown(c,200);
	if((characterCollision(c->pos)&0xFF0)){
		sfxPlay(sfxStomp,1.f);
		const vec cvec = characterGetCollisionVec(c->pos);
		c->vel = vecAdd(c->vel,vecMulS(cvec,-0.01f));
		return false;
	}
	worldSetB(los.x,los.y,los.z,b);
	if((characterCollision(c->pos)&0xFF0) != 0){
		const vec cvec = characterGetCollisionVec(c->pos);
		c->vel = vecAdd(c->vel,vecMulS(cvec,-0.01f));
		worldSetB(los.x,los.y,los.z,0);
		sfxPlay(sfxStomp,1.f);
		return false;
	} else {
		chunkDirtyRegion(los.x, los.y, los.z, 3);
		msgPlaceBlock(los.x, los.y, los.z, b);
		sfxPlay(sfxPock,1.f);
		return true;
	}
}

bool characterCheckHealth(character *c){
	if(c->hp <= 0){
		characterDie(c);
		return true;
	}
	if(c->hp > c->maxhp){
		c->hp = c->maxhp;
	}
	return false;
}

bool characterHP(character *c, int addhp){
	c->hp += addhp;
	return characterCheckHealth(c);
}

vec characterLOSBlock(const character *c, bool returnBeforeBlock) {
	const vec cv = vecMulS(vecDegToVec(c->rot),0.0625f);
	vec       cp = vecAdd(c->pos,vecNew(0,0.5,0));
	u64       l  = vecToPacked(cp);

	for(int i=0;i<64;i++){
		cp = vecAdd(cp,cv);
		const u64 ip = vecToPacked(cp);
		if(ip != l){
			if(worldGetB(cp.x,cp.y,cp.z) > 0){
				return returnBeforeBlock ? packedToVec(l) : packedToVec(ip);
			}
			l = ip;
		}
	}
	return vecNOne();
}

static u32 characterCollisionBox(const vec c, float wd, float wh){
	u32 col = 0;

	if     (checkCollision(c.x-wd,c.y,c.z   )) {col |=  0x1;}
	else if(checkCollision(c.x-wd,c.y,c.z-wh)) {col |=  0x1;}
	else if(checkCollision(c.x-wd,c.y,c.z+wh)) {col |=  0x1;}

	if     (checkCollision(c.x+wd,c.y,c.z   )) {col |=  0x2;}
	else if(checkCollision(c.x+wd,c.y,c.z-wh)) {col |=  0x2;}
	else if(checkCollision(c.x+wd,c.y,c.z+wh)) {col |=  0x2;}

	if     (checkCollision(c.x   ,c.y,c.z-wd)) {col |=  0x4;}
	else if(checkCollision(c.x-wh,c.y,c.z-wd)) {col |=  0x4;}
	else if(checkCollision(c.x+wh,c.y,c.z-wd)) {col |=  0x4;}

	if     (checkCollision(c.x   ,c.y,c.z+wd)) {col |=  0x8;}
	else if(checkCollision(c.x-wh,c.y,c.z+wd)) {col |=  0x8;}
	else if(checkCollision(c.x+wh,c.y,c.z+wd)) {col |=  0x8;}

	return col;
}

u32 characterCollision(const vec c){
	u32 col = 0;
	const float wh = 0.35f;
	const float wd = 0.4f;
	const float WD = 0.5f;

	col |= characterCollisionBox(vecAdd(c,vecNew(0, 0.9, 0)), wd, wh) << 4;

	if(checkCollision(c.x-wd,c.y-2.f ,c.z   )){col |=   0x1;}
	if(checkCollision(c.x+wd,c.y-2.f ,c.z   )){col |=   0x2;}
	if(checkCollision(c.x   ,c.y-2.f ,c.z-wd)){col |=   0x4;}
	if(checkCollision(c.x   ,c.y-2.f ,c.z+wd)){col |=   0x8;}

	if(checkCollision(c.x-WD,c.y-1.4f,c.z   )){col |= 0x100;}
	if(checkCollision(c.x+WD,c.y-1.4f,c.z   )){col |= 0x200;}
	if(checkCollision(c.x   ,c.y-1.4f,c.z-WD)){col |= 0x400;}
	if(checkCollision(c.x   ,c.y-1.4f,c.z+WD)){col |= 0x800;}

	if(checkCollision(c.x-wd,c.y-0.5f,c.z-wd)){col |= 0x1000;}
	if(checkCollision(c.x-wd,c.y-0.5f,c.z+wd)){col |= 0x2000;}
	if(checkCollision(c.x+wd,c.y-0.5f,c.z-wd)){col |= 0x4000;}
	if(checkCollision(c.x+wd,c.y-0.5f,c.z+wd)){col |= 0x8000;}

	return col;
}

u8 characterCollisionBlock(const vec c, vec *retPos){
	blockId b = 0;
	const float wd = 0.4f;
	const float WD = 0.5f;

	if((b = worldGetB(c.x-wd,c.y+1.f,c.z   ))) {*retPos = vecNew(c.x-wd,c.y+1.f,c.z   ); return b;}
	if((b = worldGetB(c.x+wd,c.y+1.f,c.z   ))) {*retPos = vecNew(c.x+wd,c.y+1.f,c.z   ); return b;}
	if((b = worldGetB(c.x   ,c.y+1.f,c.z-wd))) {*retPos = vecNew(c.x   ,c.y+1.f,c.z-wd); return b;}
	if((b = worldGetB(c.x   ,c.y+1.f,c.z+wd))) {*retPos = vecNew(c.x   ,c.y+1.f,c.z+wd); return b;}

	if((b = worldGetB(c.x-wd,c.y-2.f ,c.z   ))){*retPos = vecNew(c.x-wd,c.y-2.f,c.z   ); return b;}
	if((b = worldGetB(c.x+wd,c.y-2.f ,c.z   ))){*retPos = vecNew(c.x+wd,c.y-2.f,c.z   ); return b;}
	if((b = worldGetB(c.x   ,c.y-2.f ,c.z-wd))){*retPos = vecNew(c.x   ,c.y-2.f,c.z-wd); return b;}
	if((b = worldGetB(c.x   ,c.y-2.f ,c.z+wd))){*retPos = vecNew(c.x   ,c.y-2.f,c.z+wd); return b;}

	if((b = worldGetB(c.x-WD,c.y-1.4f,c.z   ))){*retPos = vecNew(c.x-WD,c.y-1.4f,c.z   ); return b;}
	if((b = worldGetB(c.x+WD,c.y-1.4f,c.z   ))){*retPos = vecNew(c.x+WD,c.y-1.4f,c.z   ); return b;}
	if((b = worldGetB(c.x   ,c.y-1.4f,c.z-WD))){*retPos = vecNew(c.x   ,c.y-1.4f,c.z-WD); return b;}
	if((b = worldGetB(c.x   ,c.y-1.4f,c.z+WD))){*retPos = vecNew(c.x   ,c.y-1.4f,c.z+WD); return b;}

	if((b = worldGetB(c.x-WD,c.y-0.9f,c.z-WD))){*retPos = vecNew(c.x-WD,c.y-0.9f,c.z-WD); return b;}
	if((b = worldGetB(c.x-WD,c.y-0.9f,c.z+WD))){*retPos = vecNew(c.x-WD,c.y-0.9f,c.z+WD); return b;}
	if((b = worldGetB(c.x+WD,c.y-0.9f,c.z-WD))){*retPos = vecNew(c.x+WD,c.y-0.9f,c.z-WD); return b;}
	if((b = worldGetB(c.x+WD,c.y-0.9f,c.z+WD))){*retPos = vecNew(c.x+WD,c.y-0.9f,c.z+WD); return b;}

	return 0;
}

vec characterGetCollisionVec(const vec pos){
	u32 col = characterCollision(pos);
	vec ret = vecNew(0.f,-0.8f,0.f);
	if(col & 0x110){ret.x -= 1.f;}
	if(col & 0x220){ret.x += 1.f;}
	if(col & 0x440){ret.z -= 1.f;}
	if(col & 0x880){ret.z += 1.f;}

	return ret;
}

void characterMove(character *c, const vec mov){
	const float yaw   = c->rot.yaw;

	if(c->flags & CHAR_NOCLIP){
		float s = 0.2f;
		if(c->flags & CHAR_BOOSTING){ s = 1.f;}
		c->gvel    = vecMulS(vecDegToVec(c->rot),mov.z*(-s));
		c->gvel.x += cosf((yaw)*PI/180)*mov.x*s;
		c->gvel.z += sinf((yaw)*PI/180)*mov.x*s;
		c->gvel.y += mov.y;
	}else{
		float s;
		if(c->flags & CHAR_AIMING){
			s = 0.01f;
		}else if(c->flags & CHAR_CONS_MODE){
			s = 0.025f;
		}else{
			s = 0.05f;
		}
		c->gvel.x = (cosf((yaw+90)*PI/180)*mov.z*s);
		c->gvel.z = (sinf((yaw+90)*PI/180)*mov.z*s);

		c->gvel.x += cosf((yaw)*PI/180)*mov.x*s;
		c->gvel.z += sinf((yaw)*PI/180)*mov.x*s;
		c->inaccuracy += s*78.f*vecMag(mov);
	}
}

void characterRotate(character *c, const vec rot){
	c->rot = vecAdd(c->rot,rot);
	c->inaccuracy += vecAbsSum(rot) * c->zoomFactor;

	if(c->rot.pitch < -90.f){
		 c->rot.pitch = -90.f;
	}else if(c->rot.pitch >  90.f){
		 c->rot.pitch =  90.f;
	}
	if(c->rot.yaw < 0.f){
		 c->rot.yaw += 360.f;
	}else if(c->rot.yaw >  360.f){
		 c->rot.yaw -= 360.f;
	}
}

void characterStopAnimation(character *c){
	if(c == NULL){return;}
	c->animationIndex     = 0;
	c->animationTicksLeft = 0;
}

void characterStartAnimation(character *c, animType index, int duration){
	if(c == NULL){return;}
	c->animationIndex     = index;
	c->animationTicksLeft = c->animationTicksMax = duration;
}

character *characterGetByBeing(being b){
	const uint i = beingID(b);
	if(beingType(b) != bkCharacter){ return NULL; }
	if(i >= characterCount)            { return NULL; }
	return &characterList[i];
}

being characterGetBeing(const character *c){
	if(c == NULL){return 0;}
	return beingCharacter(c - &characterList[0]);
}

character *characterClosest(const vec pos, float maxDistance){
	for(uint i=0;i<characterCount;i++){
		if(characterList[i].hp <= 0){continue;}
		const float d = vecMag(vecSub(pos,characterList[i].pos));
		if(d > maxDistance){continue;}
		return &characterList[i];
	}
	return NULL;
}

void characterAddRecoil(character *c, float recoil){
	const vec vel = vecMulS(vecDegToVec(c->rot),.01f);
	c->vel = vecAdd(c->vel, vecMulS(vel,-0.75f*recoil));
	c->rot = vecAdd(c->rot, vecNew((rngValf()-0.5f) * recoil, (rngValf()-.8f) * recoil, 0.f));
}
