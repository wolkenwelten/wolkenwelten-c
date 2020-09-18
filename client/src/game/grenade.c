#include "../game/grenade.h"
#include "../main.h"
#include "../game/entity.h"
#include "../game/character.h"
#include "../gfx/mesh.h"
#include "../gfx/effects.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

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

void grenadeExplode(vec pos, float pw, int style){
	const vec   pd  = vecSub(pos,player->pos);
	const float pdm = vecMag(pd);

	if(pdm < (16*pw*pw)){
		const float dm = sqrtf((16*pw*pw)/pdm) * -0.01f;
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

void grenadeNew(const character *ent, float pwr){
	msgNewGrenade(vecAdd(ent->pos,vecNew(0,.5,0)),ent->rot, pwr);
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
	const int index = p->val.i[7];

	for(int i=p->val.i[6];i<grenadeCount;i++){
		if(grenadeList[i].ent != NULL){
			entityFree(grenadeList[i].ent);
		}
		grenadeList[i].ent = NULL;
	}
	grenadeCount = p->val.i[6];
	if(index >= grenadeCount){return;}

	if(grenadeList[index].ent == NULL){
		grenadeList[index].ent = entityNew(vecNewP(&p->val.f[0]),vecNewP(&p->val.f[3]));
		grenadeList[index].ent->eMesh = meshBomb;
	}else{
		grenadeList[index].ent->pos = vecNewP(&p->val.f[0]);
		grenadeList[index].ent->rot = vecNewP(&p->val.f[3]);
	}
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	float x = ent->pos.x;
	float y = ent->pos.y;
	float z = ent->pos.z;

	const float mx =  0.75f;
	const float mz = -1.f;
	x += ((cos((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + cos((ent->rot.yaw)*PI/180)*mx;
	y += (sin(ent->rot.pitch*PI/180)*mz);
	z += ((sin((ent->rot.yaw+90.f)*PI/180) * cos(ent->rot.pitch*PI/180))*mz) + sin((ent->rot.yaw)*PI/180)*mx;

	for(int i=shots;i>0;i--){
		const float yaw   = ent->rot.yaw   + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		const float pitch = ent->rot.pitch + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		msgBeamBlast(vecNew(x, y, z), vecNew(yaw, pitch, 0.f), beamSize, damageMultiplier, recoilMultiplier, hitsLeft);
	}
	characterAddInaccuracy(ent,inaccuracyInc);
}
