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
#include "lightning.h"

#include "weather.h"
#include "../../game/fire.h"
#include "../../network/server.h"
#include "../../voxel/bigchungus.h"
#include "../../voxel/chungus.h"
#include "../../../../common/src/game/weather/lightning.h"
#include "../../../../common/src/misc/line.h"
#include "../../../../common/src/misc/profiling.h"
#include "../../../../common/src/network/messages.h"

#include <stdio.h>

int fireBeamSize = 0;
void lightningFireBeamCB(int x, int y, int z){
	if(worldTryB(x,y,z)){
		worldSetFire(x,y,z,fireBeamSize);
	}
}

int lightningBlockCountVal = 0;
void lightningBlockCountCB(int x, int y, int z){
	if(worldTryB(x,y,z)){lightningBlockCountVal++;}
}
void lightningBlockCount(const vec a, const vec b, int bsize){
	(void)bsize;
	lineFromTo(a.x, a.y, a.z, b.x, b.y, b.z, lightningBlockCountCB);
}

void lightningFireBeam(const vec a, const vec b, int bsize){
	fireBeamSize = 1 << (bsize + 2);
	lineFromTo(a.x, a.y, a.z, b.x, b.y, b.z, lightningFireBeamCB);
}

void tryLightningChungus(const chungus *chng){
	const int cx = ((int)chng->x << 8);
	const int cy = ((int)chng->y << 8);
	const int cz = ((int)chng->z << 8);
	if(!(chng->y & 1)){return;}

	const int lx = cx + rngValA(255);
	const int lz = cz + rngValA(255);
	const int ly = cy + 256 + 32;
	const vec lv = vecNew(lx,ly,lz);
	if(!isInClouds(lv)){return;}
	float minDist = 2048.f;
	for(uint i=0;i<clientCount;++i){
		if(clients[i].state){continue;}
		const float dist = vecMag(vecSub(clients[i].c->pos,lv));
		minDist = MIN(dist,minDist);
	}
	if(minDist > 512.f){
		return;
	}

	const int tx = cx + rngValA(255);
	const int tz = cz + rngValA(255);
	int ty;
	for(ty = cy + rngValA(255); worldTryB(tx,ty,tz); ty++){}
	if(!worldTryB(tx,--ty,tz)){return;}
	const u16 seed = rngValA((1<<16)-1);

	lightningBlockCountVal = 0;
	lightningStrike(lx,ly,lz,tx,ty,tz,seed,lightningBlockCount);
	if(lightningBlockCountVal > (stormIntensity / 2)){
		return;
	}
	msgLightningStrike(-1, lx, ly, lz, tx, ty, tz, seed);
	lightningStrike(lx,ly,lz,tx,ty,tz,seed,lightningFireBeam);
}

void tryLightning(){
	static int calls = 0;
	PROFILE_START();

	for(uint i=++calls & 0x1F;i < chungusCount; i += 0x20){
		const chungus *chng = &chungusList[i];
		if(chng->nextFree != NULL){continue;}
		//if(rngValA(0x1F)){continue;}
		tryLightningChungus(chng);
	}

	PROFILE_STOP();
}
