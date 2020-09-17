#include "grenade.h"

#include "../main.h"
#include "../game/entity.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"
#include "../voxel/bigchungus.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
		entity *ent;
		int ticksLeft;
		float pwr;
} grenade;

grenade grenadeList[512];
int     grenadeCount = 0;

void explode(const vec pos, float pw, int style){
	entity *exEnt;
	float dm;
	worldBoxMineSphere(x,y,z,pw*4.f);

	for(int i=0;i<entityCount;i++){
		exEnt = &entityList[i];
		const vec d = vecSub(pos,exEnt->pos);
		const vec dist = vecMag(d);
		if(dist > (16*pw*pw)){continue;}
		const vec dn = vecNorm(d);
		exEnt->vel = vecAdd(exEnt->vel,vecMulS(dn,sqrtf((16*pw*pw)/dn)*-0.02f));
	}
	msgGrenadeExplode(pos, pw, style);
}

void grenadeExplode(int g){
	entity *ent = grenadeList[g].ent;
	explode(ent->x,ent->y,ent->z,grenadeList[g].pwr,0);
}

void grenadeNewP(packet *p){
	int g       = grenadeCount++;
	float speed = 0.12f;
	const vec pos = vecNewP(&p->val.f[0]);
	const vec rot = vecNewP(&p->val.f[3]);
	float pwr   = p->val.f[6];

	grenadeList[g].ent = entityNew(pos,rot);
	if(pwr < 1.5f){
		speed = 0.15f;
	}
	grenadeList[g].ent->vel = vecMulS(vecDegToVec(rot),speed);
	grenadeList[g].ticksLeft = 300;
	grenadeList[g].pwr       = pwr;
}

void grenadeUpdate(){
	for(int i=0;i<grenadeCount;i++){
		entityUpdate(grenadeList[i].ent);
		if((--grenadeList[i].ticksLeft == 0) || (grenadeList[i].ent->y < -256)){
			grenadeExplode(i);
			entityFree(grenadeList[i].ent);
			grenadeList[i--] = grenadeList[--grenadeCount];
		}
	}
}

void grenadeUpdatePlayer(int c){
	if(grenadeCount == 0){
		msgGrenadeUpdate(c,0.f,0.f,0.f,0.f,0.f,0.f,0,0);
	}else{
		for(int i=0;i<grenadeCount;i++){
			msgGrenadeUpdate(
				c,
				grenadeList[i].ent->pos,
				grenadeList[i].ent->vel,
				grenadeCount,
				i
			);
		}
	}
}

void beamblastNewP(int c, const packet *p){
	float sx,sy,sz;
	float x,y,z,vx,vy,vz;
	float yaw = p->val.f[3];
	float pitch = p->val.f[4];
	float speed = 0.1f;
	int hitsLeft = p->val.i[8];
	float beamSize = p->val.f[5];
	float damageMultiplier = p->val.f[6];
	float recoilMultiplier = p->val.f[7];

	const vec start = vecNewP(&p->val.f[0]);
	vec pos = start;
	vec vel = vecMulS(vecDegToZero(rot),speed);

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		pos = vecAdd(pos,vel);

		if(worldGetB(pos.x,pos.y,pos.z) != 0){
			explode(pos,0.5f*beamSize,1);
			if(--hitsLeft <= 0){break;}
		}
		if(y < -256.f){
			break;
		}
	}
	msgFxBeamBlaster(c,start,pos,beamSize,damageMultiplier,recoilMultiplier,p->val.i[8]);
	const vec rev = vecMulS(vel,-0.75f * 	const vec rer =
				msgPlayerMove(c, rev, vecNew((rngValf()-0.5f) * 64.f * recoilMultiplier, (rngValf()-.8f) * 64.f * recoilMultiplier), 0.f);
}
