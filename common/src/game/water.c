#include "water.h"

#include "../mods/api_v1.h"
#include "../../../common/src/network/messages.h"

water waterList[1<<14];
uint  waterCount = 0;

#include <stdio.h>

void waterSendUpdate(uint c, uint i){
	water *w = &waterList[i];
	msgWaterUpdate(c,i,waterCount,w->x,w->y,w->z,w->amount);
}

void waterBox(u16 x, u16 y, u16 z, i16 w, i16 h, i16 d, i16 amount){
	for(int cx = x;cx < x+w;cx++){
	for(int cy = y;cy<y+h;cy++){
	for(int cz = z;cz<z+d;cz++){
		const u8 b = worldGetB(cx,cy,cz);
		if(b == 0){continue;}
		waterNew(cx,cy,cz,amount);
	}
	}
	}
}

water *waterGetAtPos(u16 x,u16 y, u16 z){
	for(uint i=0;i<waterCount;i++){
		water *f = &waterList[i];
		if(f->x != x){continue;}
		if(f->y != y){continue;}
		if(f->z != z){continue;}
		return f;
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
