#include "water.h"

#include "../game/being.h"
#include "../mods/api_v1.h"
#include "../../../common/src/network/messages.h"

water waterList[1<<14];
uint  waterCount = 0;

#include <stdio.h>

void waterSendUpdate(uint c, uint i){
	water *w = &waterList[i];
	msgWaterUpdate(c,i,waterCount,w->x,w->y,w->z,w->amount);
}

void waterDel(uint i){
	if(i >= waterCount){return;}
	const int  m   = waterCount - 1;
	beingList *ibl = waterList[i].bl;
	beingList *mbl = waterList[m].bl;
	being      ib  = waterGetBeing(&waterList[i]);
	being      mb  = waterGetBeing(&waterList[m]);

	beingListDel(ibl,ib);
	beingListDel(mbl,mb);
	waterList[i] = waterList[m];
	waterList[i].bl = beingListUpdate(NULL,ib);
	waterCount--;
}

void waterBox(u16 x, u16 y, u16 z, i16 w, i16 h, i16 d, i16 amount){
	for(int cx = x;cx < x+w;cx++){
	for(int cy = y;cy < y+h;cy++){
	for(int cz = z;cz < z+d;cz++){
		waterNew(cx,cy,cz,amount);
	}
	}
	}
}

water *waterGetAtPos(u16 x, u16 y, u16 z){
	beingList *bl = beingListGet(x,y,z);
	if(bl == NULL){return NULL;}
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		for(uint i=0;i<countof(ble->v);i++){
			if(beingType(ble->v[i]) != BEING_WATER){continue;}
			water *t = waterGetByBeing(ble->v[i]);
			if(t == NULL){continue;}
			if(t->x != x){continue;}
			if(t->y != y){continue;}
			if(t->z != z){continue;}
			return t;
		}
	}
	return NULL;
}

water *waterGetByBeing(being b){
	if(beingType(b) != BEING_WATER){return NULL;}
	uint i = beingID(b);
	if(i >= waterCount){return NULL;}
	return &waterList[i];
}

being waterGetBeing(const water *w){
	if(w == NULL){return 0;}
	return beingWater(w - waterList);
}
