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
#include "../../voxel/bigchungus.h"
#include "../../voxel/chungus.h"
#include "../../../../common/src/misc/profiling.h"
#include "../../../../common/src/network/messages.h"

void lightningStrikeHit(int tx, int ty, int tz){
	fireNew(tx, ty, tz,   1 << 14);
	fireNew(tx+1, ty, tz, 1 << 14);
	fireNew(tx-1, ty, tz, 1 << 14);
	fireNew(tx, ty, tz+1, 1 << 14);
	fireNew(tx, ty, tz-1, 1 << 14);
	fireNew(tx, ty-1, tz, 1 << 14);
	fireNew(tx, ty+1, tz, 1 << 14);
}

void tryLightningChungus(const chungus *chng){
	if(rngValA(63) != 0){return;}
	const int cx = ((int)chng->x << 8);
	const int cy = ((int)chng->y << 8);
	const int cz = ((int)chng->z << 8);
	if(!(chng->y & 1)){return;}

	const int lx = rngValA(255);
	const int lz = rngValA(255);
	const int ly = 256 + 32;

	const int tx = cx + rngValA(255);
	const int tz = cz + rngValA(255);
	int ty;
	for(ty = cy + rngValA(255); worldTryB(tx,ty,tz); ty++){}
	if(!worldTryB(tx,--ty,tz)){return;}

	lightningStrikeHit(tx,ty,tz);
	msgLightningStrike(-1, cx + lx, cy + ly, cz + lz, tx, ty, tz, rngValA((1<<16)-1));
}

void tryLightning(){
	static uint calls = 0;
	PROFILE_START();

	for(uint i=calls & 0x7F;i < chungusCount; i+= 0x80){
		tryLightningChungus(&chungusList[i]);
	}

	PROFILE_STOP();
	calls++;
}
