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
#include "beamblast.h"

#include "../voxel/bigchungus.h"
#include "../game/projectile.h"
#include "../../../common/src/misc/line.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include <math.h>

static void explode(const vec pos, float pw, int style){
	(void)style;
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
}

float beamblastExplosionSize = 0.f;
static void beamblastCheck(int x, int y, int z){
	if(worldTryB(x,y,z) != 0){
		explode(vecNew(x,y,z),beamblastExplosionSize,1);
	}
}

void beamblastNewP(uint c, const packet *p){
	PROFILE_START();
	float beamSize         = p->v.f[6];
	float damageMultiplier = p->v.f[7];

	const vec start = vecNewP(&p->v.f[0]);
	const vec end   = vecNewP(&p->v.f[3]);

	beamblastExplosionSize = 2.f*beamSize;
	lineFromTo(start.x, start.y, start.z, end.x, end.y, end.z, beamblastCheck);
	msgFxBeamBlaster(c,start,end,beamSize,damageMultiplier);
	PROFILE_STOP();
}
