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
#include "../../../common/src/network/messages.h"

void beamblastNewP(uint c, const packet *p){
	float beamSize         = p->v.f[6];
	float damageMultiplier = p->v.f[7];

	const vec start = vecNewP(&p->v.f[0]);
	const vec end   = vecNewP(&p->v.f[3]);

	vec pos         = start;
	vec vel         = vecNorm(vecSub(end,start));
	vec tvel        = vecMulS(vel,1.f/8.f);

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		vec spos = pos;
		for(int i=0;i<8;i++){
			spos = vecAdd(spos,tvel);
			if(worldGetB(spos.x,spos.y,spos.z) != 0){
				explode(pos,2.f*beamSize,1);
			}
		}
		pos = vecAdd(pos,vel);
	}
	msgFxBeamBlaster(c,start,pos,beamSize,damageMultiplier);
}
