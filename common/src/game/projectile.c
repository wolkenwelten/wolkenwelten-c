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

void projectileNew(const vec pos, const vec rot, being target, being source, uint style, float speed){
	const int ID = projectileNewID();
	if(ID < 0){return;}

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
}

void projectileNewC(const character *c, being target, uint style){
	const float mx =  1.f;
	const float mz = -1.f;
	vec pos = c->pos;
	pos.x += ((cos((c->rot.yaw+90.f)*PI/180) * cos(c->rot.pitch*PI/180))*mz) + cos((c->rot.yaw)*PI/180)*mx;
	pos.y += (sin(c->rot.pitch*PI/180)*mz);
	pos.z += ((sin((c->rot.yaw+90.f)*PI/180) * cos(c->rot.pitch*PI/180))*mz) + sin((c->rot.yaw)*PI/180)*mx;

	const float yaw   = c->rot.yaw   + (rngValf()-0.5f)*MIN(96.f,c->inaccuracy*0.2f);
	const float pitch = c->rot.pitch + (rngValf()-0.5f)*MIN(96.f,c->inaccuracy*0.2f);

	projectileNew(pos,vecNew(yaw,pitch,0),target,characterGetBeing(c),style,1.f);
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
	float mul = 0.13f;
	if(p->style == 3){
		mul = 0.1f;
	}
	p->vel = vecMulS(vecAdd(vecMulS(p->vel,155.f),vecMulS(norm,mul)),1.f/156.f);
	return 0;
}

int projectileSelfHitCheck(projectile *p, float mdd, being source){
	for(uint i=0;i<(sizeof(projectileList) / sizeof(projectile));i++){
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
		if(projectileSelfHitCheck(p, 2.f, projectileGetBeing(p))){return 1;}
	}
	if(characterHitCheck (p->pos, mdd, 1, 3, iteration, p->source)){return 1;}
	if(animalHitCheck    (p->pos, mdd, 1, 3, iteration, p->source)){return 1;}
	if(!vecInWorld(p->pos)){return 1;}
	if(checkCollision(p->pos.x,p->pos.y,p->pos.z)){
		if(!isClient){
			worldBoxMine(p->pos.x-1,p->pos.y,p->pos.z,3,1,1);
			worldBoxMine(p->pos.x,p->pos.y-1,p->pos.z,1,3,1);
			worldBoxMine(p->pos.x,p->pos.y,p->pos.z-1,1,1,3);
		}
		return 1;
	}
	if(p->target != 0){return projectileHomeIn(p);}
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
