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
#include "../game/item.h"
#include "../network/messages.h"
#include "../world/world.h"

fire fireList[1<<14];
uint fireCount = 0;

#include <stdio.h>
#include <stdlib.h>

void fireEmptyUpdate(uint c){
	msgFireUpdate(c,0,0,0,0,0,0);
}

void fireSendUpdate(uint c, uint i){
	fire *f = &fireList[i];
	msgFireUpdate(c,i,fireCount,f->x,f->y,f->z,f->strength);
}

void fireBox(u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, int strength){
	for(int cx = x;cx < x+w;cx++){
	for(int cy = y;cy < y+h;cy++){
	for(int cz = z;cz < z+d;cz++){
		const u8 b = worldGetB(cx,cy,cz);
		if(b == 0){continue;}
		fireNew(cx,cy,cz,strength);
	}
	}
	}
}

void fireBoxExtinguish(u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, int strength){
	if(isClient){return;}
	for(int cx = x;cx < x+w;cx++){
	for(int cy = y;cy < y+h;cy++){
	for(int cz = z;cz < z+d;cz++){
		fire *f = fireGetAtPos(cx,cy,cz);
		if(f != NULL){
			f->strength = MAX(-strength,f->strength - strength);
			fireSendUpdate(-1, f - fireList);
			if(!isClient){msgFxBeamBlastHit(-1, vecNew(f->x,f->y,f->z), 256, 2);}
		}
		if(!isClient){
			const u8 b = worldTryB(cx,cy,cz);
			if((b == I_Dry_Grass) && (rngValA(1) == 0)){
				worldSetB(cx,cy,cz,I_Grass);
			}
		}
	}
	}
	}
}

void fireDel(uint i){
	if(i >= fireCount){return;}
	const int  m   = fireCount - 1;
	beingList *ibl = fireList[i].bl;
	beingList *mbl = fireList[m].bl;
	being      ib  = fireGetBeing(&fireList[i]);
	being      mb  = fireGetBeing(&fireList[m]);

	beingListDel(ibl,ib);
	beingListDel(mbl,mb);
	fireList[i] = fireList[m];
	fireList[i].bl = beingListUpdate(NULL,ib);
	fireCount--;
}

void fireDelChungus(const chungus *c){
	if(c == NULL){return;}
	const ivec cp = chungusGetPos(c);
	for(uint i=fireCount-1;i<fireCount;i--){
		const fire *f = &fireList[i];
		if((f->x >> 8) != cp.x){continue;}
		if((f->y >> 8) != cp.y){continue;}
		if((f->z >> 8) != cp.z){continue;}
		fireDel(i);
	}
}

fire *fireGetAtPos(u16 x, u16 y, u16 z){
	beingList *bl = beingListGet(x,y,z);
	if(bl == NULL){return NULL;}
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		for(uint i=0;i<countof(ble->v);i++){
			if(beingType(ble->v[i]) != BEING_FIRE){continue;}
			fire *t = fireGetByBeing(ble->v[i]);
			if(t == NULL){continue;}
			if(t->x != x){continue;}
			if(t->y != y){continue;}
			if(t->z != z){continue;}
			return t;
		}
	}
	return NULL;
}

fire *fireGetByBeing(being b){
	if(beingType(b) != BEING_FIRE){return NULL;}
	uint i = beingID(b);
	if(i >= fireCount){return NULL;}
	return &fireList[i];
}

being fireGetBeing(const fire *f){
	if(f == NULL){return 0;}
	return beingFire(f - fireList);
}

int fireHitCheck(const vec pos, float mdd, int dmg, int cause, u16 iteration, being source){
	(void)mdd;
	(void)cause;
	(void)iteration;
	(void)source;
	fire *f = fireGetAtPos(pos.x,pos.y,pos.z);
	if(f != NULL){
		f->strength -= dmg*4;
		return 1;
	}
	return 0;
}
