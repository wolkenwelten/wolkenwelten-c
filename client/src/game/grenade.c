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

void grenadeExplode(float x, float y, float z, float pw, int style){
	const vec   pd  = vecNew(x-player->x,y-player->y,z-player->z);
	const float pdm = sqrtf(vecMag(pd));

	if(pdm < (2*pw*pw)){
		const float dm = sqrtf((2*pw*pw)/pdm) * -0.02f;
		player->vx = pd.x * dm;
		player->vy = pd.y * dm;
		player->vz = pd.z * dm;
		player->shake = dm*4.f;
	}

	for(int i=0;i<entityCount;i++){
		entity *exEnt = &entityList[i];
		const vec exd = vecNew(x-exEnt->x,y-exEnt->y,z-exEnt->z);
		const float expd = sqrtf(vecMag(exd));
		if(expd > (2*pw*pw)){continue;}
		const float dm = sqrtf((2*pw*pw)/expd) * -0.02f;
		exEnt->vx += exd.x * dm;
		exEnt->vy += exd.y * dm;
		exEnt->vz += exd.z * dm;
	}

	if(style == 0){
		fxExplosionBomb(x,y,z,pw);
	}else if(style == 1){
		fxExplosionBlaster(x,y,z,pw);
	}
}

void grenadeNew(character *ent, float pwr){
	msgNewGrenade(ent->x, ent->y+0.5f, ent->z, ent->yaw, ent->pitch, ent->roll, pwr);
}

void grenadeUpdate(){
	for(int i=0;i<grenadeCount;i++){
		entityUpdate(grenadeList[i].ent);
		grenadeList[i].ent->yaw   += 1.6f;
		grenadeList[i].ent->pitch += 1.2f;
		grenadeList[i].ent->roll  += 2.0f;
		fxGrenadeTrail(
			grenadeList[i].ent->x,
			grenadeList[i].ent->y,
			grenadeList[i].ent->z,
			grenadeList[i].pwr
		);
	}
}

void grenadeUpdateFromServer(packet *p){
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
		grenadeList[index].ent = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	}
	grenadeList[index].ent->x     = p->val.f[0];
	grenadeList[index].ent->y     = p->val.f[1];
	grenadeList[index].ent->z     = p->val.f[2];
	grenadeList[index].ent->vx    = p->val.f[3];
	grenadeList[index].ent->vy    = p->val.f[4];
	grenadeList[index].ent->vz    = p->val.f[5];
	grenadeList[index].ent->eMesh = meshBomb;
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	float x = ent->x;
	float y = ent->y;
	float z = ent->z;

	const float mx =  0.75f;
	const float mz = -1.f;
	x += ((cos((ent->yaw+90.f)*PI/180) * cos(ent->pitch*PI/180))*mz) + cos((ent->yaw)*PI/180)*mx;
	y += (sin(ent->pitch*PI/180)*mz);
	z += ((sin((ent->yaw+90.f)*PI/180) * cos(ent->pitch*PI/180))*mz) + sin((ent->yaw)*PI/180)*mx;

	for(int i=shots;i>0;i--){
		const float yaw   = ent->yaw   + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		const float pitch = ent->pitch + (rngValf()-0.5f)*ent->inaccuracy*inaccuracyMult;
		msgBeamBlast(x, y, z, yaw, pitch, beamSize, damageMultiplier, recoilMultiplier, hitsLeft);
	}
	characterAddInaccuracy(ent,inaccuracyInc);
}
