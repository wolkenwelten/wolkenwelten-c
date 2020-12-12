#include "../game/grenade.h"

#include "../game/animal.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../gfx/effects.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>

void explode(const vec pos, float pw, int style){
	const vec   pd  = vecSub(pos,player->pos);
	const float pdm = vecMag(pd);
	const float max = MAX(1,16*pw*pw);

	if(pdm < max){
		const float dm = sqrtf(max-pdm)/max * -0.1f;
		player->vel = vecAdd(player->vel,vecMulS(pd,dm));
		player->shake = dm*4.f;
	}

	for(uint i=0;i<entityCount;i++){
		entity *exEnt = &entityList[i];
		const vec exd = vecSub(pos,exEnt->pos);
		const float expd = vecMag(exd);
		if(expd > (2*pw*pw)){continue;}
		const float dm = sqrtf((2*pw*pw)/expd) * -0.02f;
		exEnt->vel = vecAdd(exEnt->vel,vecMulS(exd,dm));
	}

	if(style == 0){
		fxExplosionBomb(pos,pw);
	}else if(style == 1){
		fxExplosionBlaster(pos,pw);
	}
}

void grenadeNew(const vec pos, const vec rot, float pwr, int cluster, float clusterPwr){
	msgNewGrenade(pos, rot, pwr, cluster, clusterPwr);
}

void grenadeUpdateAll(){
	for(uint i=0;i<grenadeCount;i++){
		entityUpdate(grenadeList[i].ent);
		grenadeList[i].ent->rot = vecAdd(grenadeList[i].ent->rot,vecNew(1.6f,1.2f,2.0f));
		fxGrenadeTrail(
			grenadeList[i].ent->pos,
			grenadeList[i].pwr
		);
	}
}

void grenadeUpdateFromServer(const packet *p){
	const uint index = p->v.u16[0];
	const uint newC  = p->v.u16[1];

	for(uint i=newC;i<grenadeCount;i++){
		if(grenadeList[i].ent != NULL){
			entityFree(grenadeList[i].ent);
		}
		grenadeList[i].ent = NULL;
	}
	grenadeCount = newC;
	if(index >= grenadeCount){return;}

	if(grenadeList[index].ent == NULL){
		grenadeList[index].ent = entityNew(vecZero(),vecZero());
		grenadeList[index].ent->eMesh = meshBomb;
	}
	grenadeList[index].ent->pos = vecNewP(&p->v.f[1]);
	grenadeList[index].ent->vel = vecNewP(&p->v.f[4]);
}
