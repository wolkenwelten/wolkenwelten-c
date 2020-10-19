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

typedef struct {
	entity *ent;
	int ticksLeft;
	float pwr;
} grenade;

grenade grenadeList[512];
int     grenadeCount = 0;

void grenadeExplode(vec pos, float pw, int style){
	const vec   pd  = vecSub(pos,player->pos);
	const float pdm = vecMag(pd);
	const float max = MAX(1,16*pw*pw);

	if(pdm < max){
		const float dm = sqrtf(max-pdm)/max * -0.1f;
		player->vel = vecAdd(player->vel,vecMulS(pd,dm));
		player->shake = dm*4.f;
	}

	for(int i=0;i<entityCount;i++){
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

void grenadeNew(const character *ent, float pwr, int cluster, float clusterPwr){
	msgNewGrenade(vecAdd(ent->pos,vecNew(0,.5,0)),ent->rot, pwr, cluster, clusterPwr);
}

void grenadeUpdate(){
	for(int i=0;i<grenadeCount;i++){
		entityUpdate(grenadeList[i].ent);
		grenadeList[i].ent->rot = vecAdd(grenadeList[i].ent->rot,vecNew(1.6f,1.2f,2.0f));
		fxGrenadeTrail(
			grenadeList[i].ent->pos,
			grenadeList[i].pwr
		);
	}
}

void grenadeUpdateFromServer(const packet *p){
	const int index = p->v.u16[0];
	const int newC  = p->v.u16[1];

	for(int i=newC;i<grenadeCount;i++){
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

void singleBeamblast(character *ent, const vec start, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	static uint iteration = 0;

	float speed     = 0.1f;
	vec pos         = start;
	vec vel         = vecMulS(vecDegToVec(rot),speed);
	const float mdd = beamSize * beamSize;
	const int dmg   = ((int)damageMultiplier)+1;
	--iteration;

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		pos = vecAdd(pos,vel);
		if(worldGetB(pos.x,pos.y,pos.z) != 0){
			worldBoxSphere(pos.x,pos.y,pos.z,beamSize*2.f,0);
			worldBoxSphereDirty(pos.x,pos.y,pos.z,beamSize*2.f);
			fxExplosionBlaster(pos,beamSize/2.f);
			if(--hitsLeft <= 0){break;}
		}
		characterHitCheck(pos, mdd, dmg, 1, iteration);
		animalHitCheck   (pos, mdd, dmg, 1, iteration);

	}
	fxBeamBlaster(start,pos,beamSize,damageMultiplier);
	msgFxBeamBlaster(0,start,pos,beamSize,damageMultiplier);

	ent->vel = vecAdd(ent->vel, vecMulS(vel,-0.75f*recoilMultiplier));
	ent->rot = vecAdd(ent->rot, vecNew((rngValf()-0.5f) * 64.f * recoilMultiplier, (rngValf()-.8f) * 64.f * recoilMultiplier, 0.f));
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	const float mx =  0.75f;
	const float mz = -1.f;
	vec pos = ent->pos;
	pos.x += ((cos((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + cos((ent->rot.yaw)*PI/180)*mx;
	pos.y += (sin(ent->rot.pitch*PI/180)*mz);
	pos.z += ((sin((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + sin((ent->rot.yaw)*PI/180)*mx;

	for(int i=shots;i>0;i--){
		const float yaw   = ent->rot.yaw   + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		const float pitch = ent->rot.pitch + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		singleBeamblast(ent, pos, vecNew(yaw, pitch, 0.f), beamSize, damageMultiplier, recoilMultiplier, hitsLeft);
	}
	characterAddInaccuracy(ent,inaccuracyInc);
}
