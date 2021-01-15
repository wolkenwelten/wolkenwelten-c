#include "water.h"

#include "../game/being.h"
#include "../game/blockMining.h"
#include "../game/grenade.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include <stdio.h>

void waterNewF(u16 x, u16 y, u16 z, i16 amount){
	water *w = NULL;
	if(waterCount < countof(waterList)){
		w = &waterList[waterCount++];
	}else{
		w = &waterList[rngValM(countof(waterList))];
	}

	w->x = x;
	w->y = y;
	w->z = z;
	w->amount = amount;
	waterSendUpdate(-1,(int)(w - waterList));
}

int waterNew(u16 x, u16 y, u16 z, i16 amount){
	if(!worldIsLoaded(x,y,z)){return amount;}
	water *w = waterGetAtPos(x,y,z);
	if(w == NULL){
		if(waterCount < countof(waterList)){
			w = &waterList[waterCount++];
		}else{
			w = &waterList[rngValA(countof(waterList)-1)];
		}
		w->amount = 0;
		w->bl = NULL;
		w->x = x;
		w->y = y;
		w->z = z;
	}
	const u8 wb = worldTryB(x,y,z);
	const int ret = MIN(blocks[wb].waterCapacity,w->amount+amount) - w->amount;
	w->amount += ret;
	//waterSendUpdate(-1,(int)(w - waterList));
	w->bl = beingListUpdate(w->bl,waterGetBeing(w));
	return ret;
}

void waterRecvUpdate(uint c, const packet *p){
	(void)c;
	const u16 x      = p->v.u16[2];
	const u16 y      = p->v.u16[3];
	const u16 z      = p->v.u16[4];
	const i16 amount = p->v.i16[5];
	waterNew(x,y,z,amount);
}

static int waterFlowTo(water *w, int flowOut, int x, int y, int z){
	if(flowOut <= 0){return 0;}
	u8 wbb = worldTryB(x,y,z);

	const int flowIn = MIN(flowOut,blocks[wbb].waterIngress);
	if(flowIn <= 0){return 0;}
	w->amount -= waterNew(x,y,z,flowIn);
	return flowIn;
}

void waterUpdate(water *w){
	if(w == NULL)                  {return;}
	if((uint)(w-waterList) >= waterCount){return;}
	if((w->amount <= 0) || (w->y >= 0x8000)){
		waterDel(w-waterList);
		return waterUpdate(w);
	}
	u8  wb      = worldTryB(w->x,w->y,w->z);
	int flowOut = MIN(w->amount,blocks[wb].waterEgress);
	//printf("[%i | %i | %i] A:%i B:%i FO:%i\n",w->x,w->y,w->z,w->amount,wb,flowOut);
	flowOut -= waterFlowTo(w,flowOut,w->x,w->y-1,w->z);
	if((w->amount <= 0) || (w->y >= 0x8000)){
		waterDel(w-waterList);
		return waterUpdate(w);
	}
	if(flowOut <= 0){return;}

	if(w->amount < 8){
		if(rngValA(255) == 0){
			waterDel(w-waterList);
			return waterUpdate(w);
		}
		return;
	}

	u8 r = rngValA(3);
	u16 x = w->x;
	u16 z = w->z;
	switch(r){
	case 0: x-=1; break;
	case 1: x+=1; break;
	case 2: z-=1; break;
	case 3: z+=1; break;
	}
	flowOut = MAX(16,flowOut/2);
	waterFlowTo(w,flowOut,x,w->y,z);
	if((w->amount <= 0) || (w->y >= 0x8000)){
		waterDel(w-waterList);
		return waterUpdate(w);
	}
}

void waterUpdateAll(){
	static uint calls = 0;
	PROFILE_START();

	for(uint i=(calls&0x3F);i<waterCount;i+=0x40){
		waterUpdate(&waterList[i]);
	}
	--calls;

	PROFILE_STOP();
}

void waterSyncPlayer(uint c){
	if(waterCount == 0){return;}

	const int count = 16;
	const uint max = MIN(waterCount,clients[c].waterUpdateOffset+count);
	for(;clients[c].waterUpdateOffset<max;clients[c].waterUpdateOffset++){
		waterSendUpdate(c,clients[c].waterUpdateOffset);
	}
	if(clients[c].waterUpdateOffset >= waterCount){clients[c].waterUpdateOffset = 0;}
}

void waterDelChungus(const chungus *c){
	if(c == NULL){return;}
	const u16 cx = c->x << 8;
	const u16 cy = c->y << 8;
	const u16 cz = c->z << 8;
	for(uint i=waterCount-1;i<waterCount;i--){
		const water *w = &waterList[i];
		if((w->x & 0xFF00) != cx){continue;}
		if((w->y & 0xFF00) != cy){continue;}
		if((w->z & 0xFF00) != cz){continue;}
		waterDel(i);
	}
}
