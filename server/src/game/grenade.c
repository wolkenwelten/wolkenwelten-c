/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "grenade.h"

#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/game/projectile.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include <math.h>

void explode(const vec pos, float pw, int style){
	pw = MIN(pw,64.f);
	worldBoxMineSphere(pos.x,pos.y,pos.z,pw);

	for(uint i=0;i<entityCount;i++){
		entity *exEnt = &entityList[i];
		const vec exd = vecSub(pos,exEnt->pos);
		const float expd = vecMag(exd);
		if(expd > (2*pw)){continue;}
		const float dm = sqrtf((2*pw)/expd) * -0.05f;
		exEnt->vel = vecAdd(exEnt->vel,vecMulS(exd,dm));
	}

	for(int i=pw;i>=0;i--){
		const vec rot = vecMul(vecRng(),vecNew(180.f,90.f,0.f));
		if(projectileNew(pos, rot, 0, 0, 5, 0.07f)){break;}
	}
	msgGrenadeExplode(pos, pw, style);
}

void grenadeNew(const vec pos, const vec rot, float pwr, int cluster, float clusterPwr){
	int g       = grenadeCount++;
	float speed = 0.12f;

	grenadeList[g].ent = entityNew(pos,rot,pwr*3.f);
	if(pwr < 1.5f){
		speed = 0.15f;
	}
	grenadeList[g].ent->vel   = vecMulS(vecDegToVec(rot),speed);
	grenadeList[g].ticksLeft  = 300+(rngValR()&0x1FF);
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

void grenadeUpdateAll(){
	PROFILE_START();

	for(uint i=grenadeCount-1;i<grenadeCount;i--){
		entityUpdate(grenadeList[i].ent);
		if((--grenadeList[i].ticksLeft == 0) || (grenadeList[i].ent->pos.y < -256)){
			grenadeExplode(i);
			entityFree(grenadeList[i].ent);
			grenadeList[i] = grenadeList[--grenadeCount];
		}
	}

	PROFILE_STOP();
}

void grenadeUpdatePlayer(u8 c){
	if(grenadeCount == 0){
		if((clients[c].syncCount & 0xFF) != msgtGrenadeUpdate){return;}
		msgGrenadeUpdate(c,NULL,0,0);
	}else{
		for(uint i=0;i<grenadeCount;i++){
			msgGrenadeUpdate(c,&grenadeList[i],i,grenadeCount);
		}
	}
}
