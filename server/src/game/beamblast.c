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

#include "../game/grenade.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/line.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>

float beamblastExplosionSize = 0.f;
static void beamblastCheck(int x, int y, int z){
	if(worldTryB(x,y,z) != 0){
		printf("BLAST! %u %u %u\n",x,y,z);
		explode(vecNew(x,y,z),beamblastExplosionSize,1);
	}else{
		//worldSetB(x,y,z,3);
	}
}

// ToDo: Optimize!!!
void beamblastNewP(uint c, const packet *p){
	PROFILE_START();
	float beamSize         = p->v.f[6];
	float damageMultiplier = p->v.f[7];

	const vec start = vecNewP(&p->v.f[0]);
	const vec end   = vecNewP(&p->v.f[3]);

	beamblastExplosionSize = 2.f*beamSize;
	lineFromTo(start.x,start.y,start.z,end.x,end.y,end.z,beamblastCheck);
	msgFxBeamBlaster(c,start,end,beamSize,damageMultiplier);
	PROFILE_STOP();
}
