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

#include "fire.h"

#include "../game/being.h"
#include "../game/blockMining.h"
#include "../game/grenade.h"
#include "../game/weather.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include <stdio.h>

int windFireOffX = 0;
int windFireOffY = 0;
int windFireOffZ = 0;

void fireNewF(u16 x, u16 y, u16 z, i16 strength, i16 blockDmg, i16 oxygen){
	fire *f = NULL;
	if(fireCount < countof(fireList)){
		f = &fireList[fireCount++];
	}else{
		f = &fireList[rngValM(countof(fireList))];
	}

	f->x = x;
	f->y = y;
	f->z = z;
	f->strength = strength;
	f->blockDmg = blockDmg;
	f->oxygen   = oxygen;
	fireSendUpdate(-1,(int)(f - fireList));
	f->bl = beingListUpdate(f->bl,fireGetBeing(f));
}

void fireNew(u16 x, u16 y, u16 z, i16 strength){
	if(!inWorld(x,y,z)){return;}
	fire *f = fireGetAtPos(x,y,z);
	if(f == NULL){
		if(fireCount < countof(fireList)){
			f = &fireList[fireCount++];
		}else{
			f = &fireList[rngValM(countof(fireList))];
		}
		f->x = x;
		f->y = y;
		f->z = z;
		f->strength = 0;
		f->blockDmg = 0;
		f->oxygen   = 8;
		f->bl = beingListUpdate(NULL,fireGetBeing(f));
	}
	f->strength = MAX(f->strength,MIN(1024,f->strength+strength));
	fireSendUpdate(-1,(int)(f - fireList));
}

void fireRecvUpdate(uint c, const packet *p){
	(void)c;
	const u16 x        = p->v.u16[2];
	const u16 y        = p->v.u16[3];
	const u16 z        = p->v.u16[4];
	const i16 strength = p->v.i16[5];
	fireNew(x,y,z,strength);
}

static inline void fireSpreadToBlock(fire *f, int r,blockCategory ccat){
	u16 fx,fy,fz;
	fx = (f->x - r) + rngValM(r*2+1) + windFireOffX;
	fy = (f->y - r) + rngValM(r*2+1) + windFireOffY;
	fz = (f->z - r) + rngValM(r*2+1) + windFireOffZ;

	if((fy < f->y) && (rngValR() & 1)){
		fy = fy + (f->y-fy);
	}
	const u8 nb = worldGetB(fx,fy,fz);
	if(ccat != 0){
		blockCategory nbt = blockTypeGetCat(nb);
		if(nbt != ccat){return;}
	}
	if(nb == 0){return;}
	fireNew(fx,fy,fz,8);
	f->strength -= 8;
}

static inline void fireSpread(fire *f){
	int count = f->strength >> 7;
	for(int i=0;i<count;i++){
		if(rngValA(0x1F)){continue;}
		int r = 1;
		if((f->strength > 0xFF) && ((rngValR() & 0x07) == 0)){r = 2;}
		fireSpreadToBlock(f,r,0);
	}
	count = f->strength >> 6;
	for(int i=0;i<count+1;i++){
		if(rngValA(0x1F)){continue;}
		int r = 1;
		const uint rv = (rngValR() & 0x07);
		if(     (f->strength > 0x1F) && (rv != 0)){r = 2;}
		else if((f->strength > 0xFF) && (rv == 0)){r = 3;}
		fireSpreadToBlock(f,r,LEAVES);
	}
}

void fireUpdate(fire *f){
	if(f == NULL){return;}
	f->strength -= 2;
	if(f->strength <= 0){
		fireDel(f-fireList);
		fireSendUpdate(-1,fireCount);
		fireSendUpdate(-1,f-fireList);
		return;
	}

	int airB = 0;
	for(int x = -1;x < 2;x++){
	for(int y = -1;y < 2;y++){
	for(int z = -1;z < 2;z++){
		if(worldGetB(f->x+x,f->y+y,f->z+z) == 0){airB++;}
	}
	}
	}

	const u8 b    = worldGetB(f->x,f->y,f->z);
	const int dmg = MIN(f->oxygen,blockTypeGetFireDamage(b));

	f->strength = MIN(30000,f->strength+dmg-1);
	f->oxygen  -= dmg;
	f->oxygen  += MIN(airB,64-f->oxygen);

	if(f->strength <= 0){
		fireDel(f-fireList);
		fireSendUpdate(-1,fireCount);
		fireSendUpdate(-1,f-fireList);
		return;
	}

	if(b == 0){
		f->blockDmg = 0;
		fireSpread(f);
	}else{
		const int maxhp = blockTypeGetFireHealth(b);
		f->blockDmg = MIN(maxhp,f->blockDmg + dmg);
		if(f->blockDmg >= maxhp){
			blockMiningBurnBlock(f->x,f->y,f->z,b);
			f->blockDmg = 0;
		}
	}
	if(f->strength > 8192){
		f->strength -= 1024;
		explode(vecAdd(vecNew(f->x,f->y,f->z),vecMulS(vecRng(),4.f)), 0.5f, 0);
	}else if(f->strength > 2048){
		f->strength -= 512;
		explode(vecAdd(vecNew(f->x,f->y,f->z),vecMulS(vecRng(),3.f)), 0.3f, 0);
	}

	fireSpread(f);
}

static void fireCalcWindOff(){
	windFireOffX = 0;
	windFireOffY = 0;
	windFireOffZ = 0;
	if(windVel.x >  0.0001){windFireOffX++;}
	if(windVel.y >  0.0001){windFireOffY++;}
	if(windVel.z >  0.0001){windFireOffZ++;}
	if(windVel.x < -0.0001){windFireOffX--;}
	if(windVel.y < -0.0001){windFireOffY--;}
	if(windVel.z < -0.0001){windFireOffZ--;}
}

void fireUpdateAll(){
	static uint calls = 0;
	PROFILE_START();
	fireCalcWindOff();

	for(uint i=(calls&0x1F);i<fireCount;i+=0x20){
		fireUpdate(&fireList[i]);
	}
	calls++;

	PROFILE_STOP();
}

void fireSyncPlayer(uint c){
	if(fireCount == 0){
		fireEmptyUpdate(c);
		return;
	}

	const int count = 16;
	const uint max = MIN(fireCount,clients[c].fireUpdateOffset+count);
	for(;clients[c].fireUpdateOffset<max;clients[c].fireUpdateOffset++){
		fireSendUpdate(c,clients[c].fireUpdateOffset);
	}
	if(clients[c].fireUpdateOffset >= fireCount){clients[c].fireUpdateOffset = 0;}
}
