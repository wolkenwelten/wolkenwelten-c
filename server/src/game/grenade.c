#include "../game/grenade.h"

#include "../main.h"
#include "../network/messages.h"
#include "../game/entity.h"
#include "../../../common/src/misc.h"
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

void explode(float x, float y, float z, float pw, int style){
	entity *exEnt;
	float dm;

	worldBoxMine(x-(pw*3),y-(pw*3),z-(pw*3),(pw*6),(pw*6),(pw*6));
	worldBoxMine(x-(pw*4),y-(pw*2),z-(pw*4),(pw*8),(pw*4),(pw*8));
	worldBoxMine(x-(pw*2),y-(pw*4),z-(pw*2),(pw*4),(pw*8),(pw*4));
	for(int i=0;i<entityCount;i++){
		exEnt = &entityList[i];
		float dx = x - exEnt->x;
		float dy = y - exEnt->y;
		float dz = z - exEnt->z;

		dm = fabsf(dx);
		if(fabsf(dy) > dm){dm = fabsf(dy);}
		if(fabsf(dz) > dm){dm = fabsf(dz);}

		if(dm > (16*pw*pw)){continue;}
		dx /= dm;
		dy /= dm;
		dz /= dm;
		dm = sqrtf((16*pw*pw)/dm);
		exEnt->vx = dx * dm * -0.02f;
		exEnt->vy = dy * dm * -0.02f;
		exEnt->vz = dz * dm * -0.02f;
	}
	msgGrenadeExplode(x, y, z, pw, style);
}

void grenadeExplode(int g){
	entity *ent = grenadeList[g].ent;
	explode(ent->x,ent->y,ent->z,grenadeList[g].pwr,0);
}

void grenadeNew(packetMedium *p){
	int g       = grenadeCount++;
	float speed = 0.12f;
	float x     = p->val.f[0];
	float y     = p->val.f[1];
	float z     = p->val.f[2];
	float yaw   = p->val.f[3];
	float pitch = p->val.f[4];
	float roll  = p->val.f[5];
	float pwr   = p->val.i[6];
	//int   ID    = p->target;

	grenadeList[g].ent = entityNew(x,y,z,yaw,pitch,roll);
	if(pwr < 1.5f){
		speed = 0.15f;
	}
	grenadeList[g].ent->vx = ((cos((yaw-90.f)*PI/180) * cos(-pitch*PI/180))*speed);
	grenadeList[g].ent->vy = (sin(-pitch*PI/180)*speed);
	grenadeList[g].ent->vz = ((sin((yaw-90.f)*PI/180) * cos(-pitch*PI/180))*speed);
	grenadeList[g].ent->noRepulsion = 0;

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
	packetMedium p;
	for(int i=0;i<grenadeCount;i++){
		p.target = i;
		p.val.f[0] = grenadeList[i].ent->x;
		p.val.f[1] = grenadeList[i].ent->y;
		p.val.f[2] = grenadeList[i].ent->z;
		p.val.f[3] = grenadeList[i].ent->vx;
		p.val.f[4] = grenadeList[i].ent->vy;
		p.val.f[5] = grenadeList[i].ent->vz;
		p.val.i[6] = grenadeCount;
		packetQueueM(&p,3,c);
	}
	if(grenadeCount == 0){
		p.val.i[6] = 0;
		packetQueueM(&p,3,c);
	}
}

void beamblast(int c, const packetMedium *p){
	float sx,sy,sz;
	float x,y,z,vx,vy,vz;
	float yaw = p->val.f[3];
	float pitch = p->val.f[4];
	float speed = 0.1f;
	sx = x = p->val.f[0];
	sy = y = p->val.f[1];
	sz = z = p->val.f[2];
	vx = vy = vz = 0.f;

	vx = (cos((yaw-90.f)*PI/180) * cos(-pitch*PI/180))*speed;
	vy = sin(-pitch*PI/180)*speed;
	vz = (sin((yaw-90.f)*PI/180) * cos(-pitch*PI/180))*speed;
	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		x += vx;
		y += vy;
		z += vz;

		if(worldGetB(x,y,z) != 0){
			explode(x,y,z,0.5f,1);
		}
		if(y < -256.f){
			break;
		}
	}
	msgFxBeamBlaster(sx,sy,sz,x,y,z,1.f);
	msgPlayerMove(c, (vx * -0.75f), (vy * -0.75f), (vz * -0.75f), rngValf(), -.5f, 0.f);
}
