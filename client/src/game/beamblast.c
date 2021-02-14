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

#include "../game/animal.h"
#include "../game/character.h"
#include "../gfx/effects.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

#include <math.h>

void singleBeamblast(character *ent, const vec start, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	static u16 iteration = 0;

	vec pos         = start;
	vec vel         = vecDegToVec(rot);
	vec tvel        = vecMulS(vel,1.f/8.f);
	const float mdd = MAX(1,beamSize * beamSize);
	const int dmg   = ((int)damageMultiplier)+1;
	--iteration;

	for(int ticksLeft = 0x1FFF; ticksLeft > 0; ticksLeft--){
		vec spos = pos;
		for(int i=0;i<8;i++){
			spos = vecAdd(spos,tvel);
			if(worldGetB(spos.x,spos.y,spos.z) != 0){
				worldBoxSphere(spos.x,spos.y,spos.z,beamSize*2.f,0);
				worldBoxSphereDirty(spos.x,spos.y,spos.z,beamSize*2.f);
				fxExplosionBlaster(spos,beamSize/2.f);
				if(--hitsLeft <= 0){
					ticksLeft = 0;
					break;
				}
			}
			characterHitCheck(spos, mdd, dmg, 1, iteration, 0);
			animalHitCheck   (spos, mdd, dmg, 1, iteration, 0);
		}
		pos = vecAdd(pos,vel);
	}
	fxBeamBlaster(start,pos,beamSize,damageMultiplier);
	msgFxBeamBlaster(0,start,pos,beamSize,damageMultiplier);

	recoilMultiplier /= 1.f + (ent->aimFade * ent->zoomFactor);
	ent->vel = vecAdd(ent->vel, vecMulS(vel,-0.75f*recoilMultiplier));
	ent->rot = vecAdd(ent->rot, vecNew((rngValf()-0.5f) * 64.f * recoilMultiplier, (rngValf()-.8f) * 64.f * recoilMultiplier, 0.f));
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	const float mx =  1.f - ent->aimFade;
	const float mz = -1.f;
	vec pos = ent->pos;
	pos.x += ((cosf((ent->rot.yaw+90.f)*PI/180) * cosf(ent->rot.pitch*PI/180))*mz) + cosf((ent->rot.yaw)*PI/180)*mx;
	pos.y += (sinf(ent->rot.pitch*PI/180)*mz);
	pos.z += ((sinf((ent->rot.yaw+90.f)*PI/180) * cosf(ent->rot.pitch*PI/180))*mz) + sinf((ent->rot.yaw)*PI/180)*mx;

	for(int i=shots;i>0;i--){
		const float inacc = MIN(96.f,(ent->inaccuracy*inaccuracyMult)) / (1.f + (ent->aimFade * ent->zoomFactor));
		const float yaw   = ent->rot.yaw   + (rngValf()-0.5f)*inacc;
		const float pitch = ent->rot.pitch + (rngValf()-0.5f)*inacc;
		singleBeamblast(ent, pos, vecNew(yaw, pitch, 0.f), beamSize, damageMultiplier, recoilMultiplier, hitsLeft);
	}
	characterAddInaccuracy(ent,inaccuracyInc);
}
