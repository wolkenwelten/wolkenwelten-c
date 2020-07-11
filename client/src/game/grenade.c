#include "../game/grenade.h"
#include "../main.h"
#include "../game/entity.h"
#include "../game/character.h"
#include "../gfx/mesh.h"
#include "../gfx/effects.h"
#include "../gfx/objs.h"
#include "../../../common/src/misc.h"
#include "../network/messages.h"
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


void grenadeExplode(float x, float y, float z, float pw, int style){
	worldBox(x-(pw*3),y-(pw*3),z-(pw*3),(pw*6),(pw*6),(pw*6),0);
	worldBox(x-(pw*4),y-(pw*2),z-(pw*4),(pw*8),(pw*4),(pw*8),0);
	worldBox(x-(pw*2),y-(pw*4),z-(pw*2),(pw*4),(pw*8),(pw*4),0);

	float dx = x - player->x;
	float dy = y - player->y;
	float dz = z - player->z;
	float dm = fabsf(dx);
	if(fabsf(dy) > dm){dm = fabsf(dy);}
	if(fabsf(dz) > dm){dm = fabsf(dz);}

	if(dm < (16*pw*pw)){
		dx /= dm;
		dy /= dm;
		dz /= dm;
		dm = sqrtf((16*pw*pw)/dm);
		player->vx = dx * dm * -0.02f;
		player->vy = dy * dm * -0.02f;
		player->vz = dz * dm * -0.02f;
		player->shake = dm*4.f;
	}

	if(style == 0){
		fxExplosionBomb(x,y,z,pw);
	}else if(style == 1){
		fxExplosionBlaster(x,y,z,pw);
	}
}

void grenadeNew(character *ent, float pwr){
	int ID = 256;
	if(pwr > 1.5f){ID = 257;}
	msgNewGrenade(ID, ent->x, ent->y+0.5f, ent->z, ent->yaw, ent->pitch, ent->roll, pwr);
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

void grenadeUpdateFromServer(packetMedium *p){
	for(int i=p->val.i[6];i<grenadeCount;i++){
		if(grenadeList[i].ent != NULL){
			entityFree(grenadeList[i].ent);
		}
		grenadeList[i].ent = NULL;
	}
	grenadeCount = p->val.i[6];
	if(p->target >= grenadeCount){return;}

	if(grenadeList[p->target].ent == NULL){
		grenadeList[p->target].ent = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	}
	grenadeList[p->target].ent->x     = p->val.f[0];
	grenadeList[p->target].ent->y     = p->val.f[1];
	grenadeList[p->target].ent->z     = p->val.f[2];
	grenadeList[p->target].ent->vx    = p->val.f[3];
	grenadeList[p->target].ent->vy    = p->val.f[4];
	grenadeList[p->target].ent->vz    = p->val.f[5];
	grenadeList[p->target].ent->eMesh = meshBomb;
}

void beamblast(character *ent, float pwr, int hitsLeft){
	msgBeamBlast(hitsLeft, ent->x, ent->y, ent->z, ent->yaw, ent->pitch, ent->roll, pwr);
}
