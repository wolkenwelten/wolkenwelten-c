#include "projectile.h"

#include "../game/animal.h"
#include "../game/being.h"
#include "../game/character.h"
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
		if(projectileList[i].style == 0){return i;}
	}
	return -1;
}

void projectileNew(const vec pos, const vec rot, being target, being source, uint style){
	const int ID = projectileNewID();
	if(ID < 0){return;}

	uint ttl = 512;
	if(style == 2){ttl = 4096;}
	projectile *p = &projectileList[ID];
	p->pos    = pos;
	p->target = target;
	p->source = source;
	p->style  = style;
	p->ttl    = ttl;
	p->vel    = vecMulS(vecDegToVec(rot),1.f);

	projectileSendUpdate(-1,ID);
}

void projectileNewC(const character *c, being target, uint style){
	const float mx =  1.f;
	const float mz = -1.f;
	vec pos = c->pos;
	pos.x += ((cos((c->rot.yaw+90.f)*PI/180) * cos(c->rot.pitch*PI/180))*mz) + cos((c->rot.yaw)*PI/180)*mx;
	pos.y += (sin(c->rot.pitch*PI/180)*mz);
	pos.z += ((sin((c->rot.yaw+90.f)*PI/180) * cos(c->rot.pitch*PI/180))*mz) + sin((c->rot.yaw)*PI/180)*mx;

	const float yaw   = c->rot.yaw   + (rngValf()-0.5f)*c->inaccuracy*0.2f;
	const float pitch = c->rot.pitch + (rngValf()-0.5f)*c->inaccuracy*0.2f;

	projectileNew(pos,vecNew(yaw,pitch,0),target,characterGetBeing(c),style);
}

int projectileGetClient(uint i){
	if(i > 8192){return -1;}
	return i >> 8;
}

static inline void projectileHomeIn(projectile *p){
	const vec tpos = beingGetPos(p->target);
	const vec dist = vecNorm(vecSub(tpos,p->pos));
	p->vel = vecMulS(vecAdd(vecMulS(p->vel,127.f),dist),1.f/156.f);
}

int projectileHitCheck(const vec pos, float mdd, int dmg, int cause, u16 iteration, being source){
	(void)dmg;
	(void)cause;
	(void)source;
	(void)iteration;
	for(uint i=0;i<(sizeof(projectileList) / sizeof(projectile));i++){
		if(projectileList[i].style == 0){continue;}
		if(beingProjectile(i) == source){continue;}
		const vec d = vecSub(pos,projectileList[i].pos);
		if(vecDot(d,d) < mdd){return 1;}
	}
	return 0;
}

#include <stdio.h>
static inline int projectileUpdate(projectile *p){
	static uint iteration = 0;
	if(--p->ttl < 0){return 1;}
	p->pos = vecAdd(p->pos,p->vel);
	--iteration;
	float mdd = 1.f;
	if(p->target != 0){
		mdd = 0.08f;
		if(projectileHitCheck(p->pos, 2.f, 1, 3, iteration, projectileGetBeing(p))){return 1;}
	}
	if(characterHitCheck (p->pos, mdd, 1, 3, iteration, p->source)){return 1;}
	if(animalHitCheck    (p->pos, mdd, 1, 3, iteration, p->source)){return 1;}
	if(!vecInWorld(p->pos)){return 1;}
	if(checkCollision(p->pos.x,p->pos.y,p->pos.z)){
		if(!isClient){worldBoxMine(p->pos.x,p->pos.y,p->pos.z,1,1,1);}
		return 1;
	}
	if(p->target != 0){projectileHomeIn(p);}
	return 0;
}

void projectileUpdateAll(){
	for(uint i=0;i<(sizeof(projectileList) / sizeof(projectile));i++){
		if(projectileList[i].style == 0){continue;}
		if(projectileUpdate(&projectileList[i])){
			projectileList[i].style = 0;
			if(!isClient){
				msgFxBeamBlastHit(-1, projectileList[i].pos, 256, 1);
				projectileSendUpdate(-1,i);
			}
		}
	}
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

	packetQueue(p,38,10*4,c);
}

void projectileRecvUpdate(uint c, const packet *p){
	uint i = p->v.u16[0];
	if(i > (sizeof(projectileList) / sizeof(projectile))) {return;}
	if(isClient){
		if((i >> 8) == (uint)playerID)                {return;}
	}else{
		if((i >> 8) != c)                             {return;}
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
