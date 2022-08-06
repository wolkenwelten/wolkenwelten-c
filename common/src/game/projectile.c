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

#include "../game/being.h"
#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/fire.h"
#include "../misc/profiling.h"
#include "../world/world.h"

#include <math.h>
#include <string.h>

projectile projectileList[8192];

void projectileInit(){
	memset(projectileList, 0, sizeof(projectileList));
}

int projectileNewID(){
	return 0;
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
	p->ttl    = ttl*4;
	p->vel    = vecMulS(vecDegToVec(rot),speed*0.25f);
	return false;
}

bool projectileNewC(const character *c, being target, uint style){
	const float mx =  1.f;// - c->aimFade;
	const float mz = -1.f;
	vec pos = c->pos;
	pos.x += ((cosf((c->rot.yaw+90.f)*PI/180) * cosf(c->rot.pitch*PI/180))*mz) + cosf((c->rot.yaw)*PI/180)*mx;
	pos.y += (sinf(c->rot.pitch*PI/180)*mz);
	pos.z += ((sinf((c->rot.yaw+90.f)*PI/180) * cosf(c->rot.pitch*PI/180))*mz) + sinf((c->rot.yaw)*PI/180)*mx;

	const float inacc = MIN(96.f,(c->inaccuracy*0.2f)) / c->zoomFactor;
	const float yaw   = c->rot.yaw   + (rngValf()-0.5f)*inacc;
	const float pitch = c->rot.pitch + (rngValf()-0.5f)*inacc;
	float speed = 0.5f;
	if(style == 5){
		speed = 0.1f;
	}else if(style == 6){
		speed = 0.2f;
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

static int projectileBounce(projectile *p, u8 b){
	blockCategory bc = blockTypeGetCat(b);
	if(bc != STONE){return 1;}
	const float px  = (p->pos.x - floorf(p->pos.x))-0.5f;
	const float py  = (p->pos.y - floorf(p->pos.y))-0.5f;
	const float pz  = (p->pos.z - floorf(p->pos.z))-0.5f;
	const float apx = fabsf(px);
	const float apy = fabsf(py);
	const float apz = fabsf(pz);
	if((apx > apy) && (apx > apz)){
		p->vel.x *= -0.98f;
	}
	if((apy > apx) && (apy > apz)){
		p->vel.y *= -0.98f;
	}
	if((apz > apy) && (apz > apx)){
		p->vel.z *= -0.98f;
	}
	p->vel = vecMulS(p->vel,0.8f);
	p->pos = vecAdd(p->pos,p->vel);
	p->pos = vecAdd(p->pos,p->vel);
	p->source = 0;
	if(checkCollision(p->pos.x,p->pos.y,p->pos.z)){return 1;}
	return 0;
}

static inline int projectileUpdate(projectile *p){
	static uint iteration = 0;
	if(--p->ttl < 0){return 1;}
	p->pos    = vecAdd(p->pos,p->vel);
	p->vel.y -= 0.00003f;
	--iteration;

	if(p->target != 0){
		if(projectileSelfHitCheck(p, 2.f, projectileGetBeing(p))){
			return 1;
		}
	}
	if(!vecInWorld(p->pos)){return 1;}
	const blockId b = worldTryB(p->pos.x, p->pos.y, p->pos.z);
	if(b){return projectileBounce(p,b);}
        if(p->style == 6){
                if(worldTryFire(p->pos.x, p->pos.y, p->pos.z)){return 1;}
                if(worldTryFluid(p->pos.x, p->pos.y, p->pos.z)){return 1;}
        }
	if(p->target != 0){return projectileHomeIn(p);}
	return 0;
}

static void projectileStep(projectile *p){
	if(p->style == 0){return;}
	if(projectileUpdate(p)){
		if(p->style == 6){
			fireBoxExtinguish(p->pos.x-1,p->pos.y-1,p->pos.z-1,3,3,3, 128);
		}else if(p->style == 5){
			fireBox(p->pos.x-1,p->pos.y-1,p->pos.z-1,3,3,3,64);
		}else{
			fireBox(p->pos.x,p->pos.y,p->pos.z,1,1,1,64);
		}
		p->style = 0;
		if(isClient){return;}
	}
}

void projectileUpdateAll(){
	PROFILE_START();
	for(uint i=0;i<countof(projectileList);i++){
		projectile *p = &projectileList[i];
		projectileStep(p);
		projectileStep(p);
		projectileStep(p);
		projectileStep(p);
	}
	PROFILE_STOP();
}

being projectileGetBeing (const projectile *p){
	return beingProjectile(p-projectileList);
}
