#include "fire.h"

#include "../mods/api_v1.h"
#include "../game/being.h"
#include "../../../common/src/network/messages.h"

fire fireList[1<<14];
uint fireCount = 0;

#include <stdio.h>
#include <stdlib.h>

void fireSendUpdate(uint c, uint i){
	fire *f = &fireList[i];
	msgFireUpdate(c,i,fireCount,f->x,f->y,f->z,f->strength);
}

void fireBox(int x, int y, int z, int w, int h, int d, int strength){
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

void fireBoxExtinguish(int x, int y, int z, int w, int h, int d, int strength){
	if(isClient){return;}
	for(int cx = x;cx < x+w;cx++){
	for(int cy = y;cy < y+h;cy++){
	for(int cz = z;cz < z+d;cz++){
		fire *f = fireGetAtPos(cx,cy,cz);
		if(f == NULL){continue;}
		f->strength = MAX(-strength,f->strength - strength);
		fireSendUpdate(-1, f - fireList);
		if(!isClient){msgFxBeamBlastHit(-1, vecNew(f->x,f->y,f->z), 256, 2);}
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
