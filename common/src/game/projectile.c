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

#include "projectile.h"

#include "../game/animal.h"
#include "../game/being.h"
#include "../game/character.h"
#include "../game/fire.h"
#include "../misc/profiling.h"
#include "../mods/api_v1.h"
#include "../network/packet.h"
#include "../network/messages.h"

#include <math.h>

projectile projectileList[8192];

int projectileNewID(){
	int ID = playerID;
	if(ID < 0){return -1;}
	if(ID == 64){ID = 31;} // ToDo: make this a bit nicer
	const int start = ID << 8;
	for(int i = start;i<start+256;i++){
		if(projectileList[i].style == 0){return  i;}
	}
	return -1;
}

bool projectileNew(const vec pos, const vec rot, being target, being source, uint style, float speed){
	const int ID = projectileNewID();
	if(ID < 0){return true;}

	uint ttl = 512;
	if(target != 0){ttl = 4096;}
	projectile *p = &projectileList[ID];
	p->pos    = pos;
	p->target = target;
	p->source = source;
	p->style  = style;
	p->ttl    = ttl;
	p->vel    = vecMulS(vecDegToVec(rot),speed);

	projectileSendUpdate(-1,ID);
	return false;
}

bool projectileNewC(const character *c, being target, uint style){
	const float mx =  1.f - c->aimFade;
	const float mz = -1.f;
	vec pos = c->pos;
	pos.x += ((cosf((c->rot.yaw+90.f)*PI/180) * cosf(c->rot.pitch*PI/180))*mz) + cosf((c->rot.yaw)*PI/180)*mx;
	pos.y += (sinf(c->rot.pitch*PI/180)*mz);
	pos.z += ((sinf((c->rot.yaw+90.f)*PI/180) * cosf(c->rot.pitch*PI/180))*mz) + sinf((c->rot.yaw)*PI/180)*mx;

	const float inacc = MIN(96.f,(c->inaccuracy*0.2f)) / (1.f + (c->aimFade * c->zoomFactor));
	const float yaw   = c->rot.yaw   + (rngValf()-0.5f)*inacc;
	const float pitch = c->rot.pitch + (rngValf()-0.5f)*inacc;
	float speed = 2.f;
	if(style == 5){
		speed = 0.4f;
	}else if(style == 6){
		speed = 0.5f;
	}

	return projectileNew(pos,vecNew(yaw,pitch,0),target,characterGetBeing(c),style,speed);
}

int projectileGetClient(uint i){
	if(i > 8192){return -1;}
	return i >> 8;
}

static inline int projectileHomeIn(projectile *p){
	const vec tpos = beingGetPos(p->target);
	const vec dist = vecSub(tpos,p->pos);
	const vec norm = vecNorm(dist);
	if(!isClient && (p->style == 3) && vecDot(dist,dist) < (16*16)){
		for(int i=0;i<8;i++){
			const vec rot = vecNew(rngValf()*360.f,(rngValf()*180)-90.f,0.f);
			projectileNew(p->pos, rot, p->target, p->source, 2, 0.05f);
		}
		msgFxBeamBlastHit(-1, p->pos, 256, 1);
		return 1;
	}
	float mul;
	if(p->style == 3){
		mul = 0.02f;
	}else {
		mul = 0.04f;
	}
	p->vel = vecMulS(vecAdd(vecMulS(p->vel,15.f),vecMulS(norm,mul)),1.f/16.f);
	return 0;
}

int projectileSelfHitCheck(projectile *p, float mdd, being source){
	for(uint i=0;i<countof(projectileList);i++){
		if(projectileList[i].style == 0){continue;}
		if(projectileList[i].style == 2){continue;}
		if(beingProjectile(i) == source){continue;}
		const vec d = vecSub(p->pos,projectileList[i].pos);
		if(vecDot(d,d) < mdd){
			return 1;
		}
	}
	return 0;
}

static inline int projectileUpdate(projectile *p){
	static uint iteration = 0;
	if(--p->ttl < 0){return 1;}
	p->pos    = vecAdd(p->pos,p->vel);
	p->vel.y -= 0.0005f;
	--iteration;
	float mdd = 1.f;

	if(p->target != 0){
		mdd = 1.f;
		if(projectileSelfHitCheck(p, 2.f, projectileGetBeing(p))){
			msgFxBeamBlastHit(-1, p->pos, 1, 3);
			return 1;
		}
	}
	if(!vecInWorld(p->pos)){return 1;}
	if(characterHitCheck (p->pos, mdd, 1, 3, iteration, p->source)){return 1;}
	if(animalHitCheck    (p->pos, mdd, 1, 3, iteration, p->source)){return 1;}
	if((p->style == 6) && fireHitCheck(p->pos, mdd, 1, 3, iteration, p->source)){return 1;}
	if(checkCollision(p->pos.x,p->pos.y,p->pos.z)){return 1;}
	if(p->target != 0){return projectileHomeIn(p);}
	return 0;
}

void projectileUpdateAll(){
	PROFILE_START();

	for(uint i=0;i<countof(projectileList);i++){
		projectile *p = &projectileList[i];
		if(p->style == 0){continue;}
		if(projectileUpdate(p)){
			if(p->style == 6){
				fireBoxExtinguish(p->pos.x-1,p->pos.y-1,p->pos.z-1,3,3,3, 128);
				if(!isClient){msgFxBeamBlastHit(-1, p->pos, 256, 2);}
			}else if(p->style == 5){
				fireBox(p->pos.x-1,p->pos.y-1,p->pos.z-1,3,3,3,64);
				if(!isClient){msgFxBeamBlastHit(-1, p->pos, 256, 1);}
			}else{
				fireBox(p->pos.x,p->pos.y,p->pos.z,1,1,1,64);
				if(!isClient){msgFxBeamBlastHit(-1, p->pos, 256, 1);}
			}
			projectileList[i].style = 0;
			if(!isClient){
				projectileSendUpdate(-1,i);
			}
		}
	}

	PROFILE_STOP();
}

void projectileSendUpdate(uint c, uint i){
	projectile *a = &projectileList[i];
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = a->style;
	p->v.i16[2] = a->ttl;
	p->v.i16[3] = 0;

	p->v.u32[2] = a->source;
	p->v.u32[3] = a->target;

	p->v.f  [4] = a->pos.x;
	p->v.f  [5] = a->pos.y;
	p->v.f  [6] = a->pos.z;

	p->v.f  [7] = a->vel.x;
	p->v.f  [8] = a->vel.y;
	p->v.f  [9] = a->vel.z;

	packetQueue(p,msgtProjectileUpdate,10*4,c);
}

void projectileRecvUpdate(uint c, const packet *p){
	uint i = p->v.u16[0];
	if(i > countof(projectileList)){return;}
	if(isClient){
		if((i >> 8) == (uint)playerID){return;}
	}else{
		if((i >> 8) != c){return;}
	}

	projectile *a = &projectileList[i];

	a->style  = p->v.u16[1];
	a->ttl    = p->v.i16[2];
	a->source = p->v.u32[2];
	a->target = p->v.u32[3];

	a->pos = vecNewP(&p->v.f[4]);
	a->vel = vecNewP(&p->v.f[7]);

	if(!isClient){projectileSendUpdate(-1,i);}
}

being projectileGetBeing (const projectile *p){
	return beingProjectile(p-projectileList);
}
