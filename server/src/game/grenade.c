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
	worldBoxMineSphere(pos.x,pos.y,pos.z,pw*4.f);

	for(int i=0;i<entityCount;i++){
		exEnt = &entityList[i];
		const vec d      = vecSub(pos,exEnt->pos);
		const float dist = vecMag(d);
		if(dist > (16*pw*pw)){continue;}
		const vec dn = vecNorm(d);
		exEnt->vel = vecAdd(exEnt->vel,vecMulS(dn,sqrtf((16*pw*pw)/dist)*-0.02f));
	}
	msgGrenadeExplode(pos, pw, style);
}

void grenadeExplode(int g){
	entity *ent = grenadeList[g].ent;
	explode(ent->pos,grenadeList[g].pwr,0);
}

void grenadeNewP(const packet *p){
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
		if((--grenadeList[i].ticksLeft == 0) || (grenadeList[i].ent->pos.y < -256)){
			grenadeExplode(i);
			entityFree(grenadeList[i].ent);
			grenadeList[i--] = grenadeList[--grenadeCount];
		}
	}
}

void grenadeUpdatePlayer(int c){
	if(grenadeCount == 0){
		msgGrenadeUpdate(c,vecZero(),vecZero(),0,0);
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
	float yaw = p->val.f[3];
	float pitch = p->val.f[4];
	float speed = 0.1f;
	int hitsLeft = p->val.i[8];
	float beamSize = p->val.f[5];
	float damageMultiplier = p->val.f[6];
	float recoilMultiplier = p->val.f[7];

	const vec start = vecNewP(&p->val.f[0]);
	vec pos = start;
	const vec rot   = vecNew(yaw,pitch,0);
	vec vel = vecMulS(vecDegToVec(rot),speed);

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		pos = vecAdd(pos,vel);

		if(worldGetB(pos.x,pos.y,pos.z) != 0){
			explode(pos,0.5f*beamSize,1);
			if(--hitsLeft <= 0){break;}
		}
		if(pos.y < -256.f){
			break;
		}
	}
	msgFxBeamBlaster(c,start,pos,beamSize,damageMultiplier,recoilMultiplier,p->val.i[8]);
	const vec rev = vecMulS(vel,-0.75f*recoilMultiplier);
	msgPlayerMove(c, rev, vecNew((rngValf()-0.5f) * 64.f * recoilMultiplier, (rngValf()-.8f) * 64.f * recoilMultiplier, 0.f));
}
