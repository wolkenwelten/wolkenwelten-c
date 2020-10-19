#include "grenade.h"

#include "../game/entity.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>

typedef struct {
	entity *ent;
	int     ticksLeft,cluster;
	float   pwr,clusterPwr;
} grenade;

grenade grenadeList[512];
uint    grenadeCount = 0;

void explode(const vec pos, float pw, int style){
	entity *exEnt;
	worldBoxMineSphere(pos.x,pos.y,pos.z,pw*4.f);

	for(uint i=0;i<entityCount;i++){
		exEnt = &entityList[i];
		const vec d      = vecSub(pos,exEnt->pos);
		const float dist = vecMag(d);
		if(dist > (16*pw*pw)){continue;}
		const vec dn = vecNorm(d);
		exEnt->vel = vecAdd(exEnt->vel,vecMulS(dn,sqrtf((16*pw*pw)/dist)*-0.02f));
	}
	msgGrenadeExplode(pos, pw, style);
}

static void grenadeNew(const vec pos, const vec rot, float pwr, int cluster, float clusterPwr){
	int g         = grenadeCount++;
	float speed   = 0.12f;

	grenadeList[g].ent = entityNew(pos,rot);
	if(pwr < 1.5f){
		speed = 0.15f;
	}
	grenadeList[g].ent->vel   = vecMulS(vecDegToVec(rot),speed);
	grenadeList[g].ticksLeft  = 300;
	grenadeList[g].pwr        = pwr;
	grenadeList[g].cluster    = cluster;
	grenadeList[g].clusterPwr = clusterPwr;
}

static void grenadeCluster(const grenade *g){
	if(g->cluster <= 0){return;}
	for(int i=0;i<MIN(256,g->cluster);i++){
		vec rot = vecZero();
		rot.x = rngValf()*360.f;
		rot.y = -(rngValf()*45.f);
		grenadeNew(g->ent->pos,rot,g->clusterPwr,0,0);
	}
}

void grenadeExplode(uint g){
	entity *ent = grenadeList[g].ent;
	explode(ent->pos,grenadeList[g].pwr,0);
	grenadeCluster(&grenadeList[g]);
}

void grenadeNewP(const packet *p){
	grenadeNew(vecNewP(&p->v.f[0]),vecNewP(&p->v.f[3]),p->v.f[6],p->v.i32[7],p->v.f[8]);
}

void grenadeUpdate(){
	for(uint i=grenadeCount-1;i<grenadeCount;i--){
		entityUpdate(grenadeList[i].ent);
		if((--grenadeList[i].ticksLeft == 0) || (grenadeList[i].ent->pos.y < -256)){
			grenadeExplode(i);
			entityFree(grenadeList[i].ent);
			grenadeList[i] = grenadeList[--grenadeCount];
		}
	}
}

void grenadeUpdatePlayer(uint c){
	if(grenadeCount == 0){
		msgGrenadeUpdate(c,vecZero(),vecZero(),0,0);
	}else{
		for(uint i=0;i<grenadeCount;i++){
			msgGrenadeUpdate(
				c,
				grenadeList[i].ent->pos,
				grenadeList[i].ent->vel,
				i,
				grenadeCount
			);
		}
	}
}

void beamblastNewP(uint c, const packet *p){
	float speed            = 0.1f;
	float beamSize         = p->v.f[6];
	float damageMultiplier = p->v.f[7];

	const vec start = vecNewP(&p->v.f[0]);
	const vec end   = vecNewP(&p->v.f[3]);
	const vec vel   = vecMulS(vecNorm(vecSub(end,start)),speed);
	vec pos = start;

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		pos = vecAdd(pos,vel);

		if(worldGetB(pos.x,pos.y,pos.z) != 0){
			explode(pos,0.5f*beamSize,1);
		}
	}
	msgFxBeamBlaster(c,start,pos,beamSize,damageMultiplier);
}
