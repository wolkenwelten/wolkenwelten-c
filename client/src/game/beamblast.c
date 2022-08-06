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

#include "../game/character/character.h"
#include "../game/entity.h"
#include "../game/projectile.h"
#include "../gfx/effects.h"
#include "../voxel/bigchungus.h"

#include <math.h>

void explode(const vec pos, float pw, int style){
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

void singleBeamblast(const vec start, const vec rot, float beamSize, float damageMultiplier, int hitsLeft){
	static u16 iteration = 0;

	vec pos         = start;
	vec vel         = vecDegToVec(rot);
	vec tvel        = vecMulS(vel,1.f/2.f);
	//const float mdd = MAX(1,beamSize * beamSize);
	//const int dmg   = ((int)damageMultiplier)+1;
	//being source    = characterGetBeing(player);
	--iteration;

	for(int ticksLeft = 0x7FF; ticksLeft > 0; ticksLeft--){
		vec spos = pos;
		for(int i=0;i<8;i++){
			spos = vecAdd(spos,tvel);
			if(worldGetB(spos.x,spos.y,spos.z) != 0){
				worldBoxSphere(spos.x,spos.y,spos.z,beamSize*2.f,0);
				worldBoxSphereDirty(spos.x,spos.y,spos.z,beamSize*2.f);
				fxExplosionBlaster(spos,beamSize/2.f);
				if(--hitsLeft <= 0){
					ticksLeft = 0;
					pos = vecAdd(pos,vecMulS(vel,4.f));
					break;
				}
			}
			//characterHitCheck(spos, mdd, dmg, 1, iteration, source);
		}
		pos = vecAdd(pos,vel);
	}
	fxBeamBlaster(  start,pos,beamSize,damageMultiplier);
}

void beamblast(character *ent, float beamSize, float damageMultiplier, int hitsLeft){
	const float mx =  1.f;// - ent->aimFade;
	const float mz = -1.f;
	vec pos = ent->pos;
	pos.x += ((cosf((ent->rot.yaw+90.f)*PI/180) * cosf(ent->rot.pitch*PI/180))*mz) + cosf((ent->rot.yaw)*PI/180)*mx;
	pos.y += (sinf(ent->rot.pitch*PI/180)*mz);
	pos.z += ((sinf((ent->rot.yaw+90.f)*PI/180) * cosf(ent->rot.pitch*PI/180))*mz) + sinf((ent->rot.yaw)*PI/180)*mx;

	const float inacc = MIN(96.f,(ent->inaccuracy)) / ent->zoomFactor;
	const float yaw   = ent->rot.yaw   + (rngValf()-0.5f)*inacc;
	const float pitch = ent->rot.pitch + (rngValf()-0.5f)*inacc;
	singleBeamblast(pos, vecNew(yaw, pitch, 0.f), beamSize, damageMultiplier, hitsLeft);
}
