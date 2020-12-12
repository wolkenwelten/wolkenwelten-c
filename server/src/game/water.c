#include "water.h"

#include "../game/blockMining.h"
#include "../game/grenade.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

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

void waterNew(u16 x, u16 y, u16 z, i16 amount){
	water *w = NULL;

	for(uint i=waterCount-1;i<waterCount;i--){
		if(waterList[i].x != x){continue;}
		if(waterList[i].y != y){continue;}
		if(waterList[i].z != z){continue;}
		w = &waterList[i];
		break;
	}
	if(w == NULL){
		if(waterCount < countof(waterList)){
			w = &waterList[waterCount++];
		}else{
			w = &waterList[rngValM(countof(waterList))];
		}
		w->amount = 0;
	}

	w->x = x;
	w->y = y;
	w->z = z;
	w->amount += amount;
	waterSendUpdate(-1,(int)(w - waterList));
}

void waterRecvUpdate(uint c, const packet *p){
	(void)c;
	const u16 x      = p->v.u16[2];
	const u16 y      = p->v.u16[3];
	const u16 z      = p->v.u16[4];
	const i16 amount = p->v.i16[5];
	waterNew(x,y,z,amount);
}

void waterUpdate(water *w){
	printf("w1\n");
	if(w == NULL){return;}
	printf("w2\n");
	if(blockTypeGetWaterImpermeable(worldGetB(w->x,w->y,w->z))){return;}
	if(!blockTypeGetWaterImpermeable(worldGetB(w->x,w->y-1,w->z))){
		waterNew(w->x,w->y-1,w->z,w->amount/2);
		w->amount /= 2;
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
	if(!blockTypeGetWaterImpermeable(worldGetB(x,w->y,z))){
		waterNew(x,w->y,z,w->amount/4);
		w->amount /= 4;
		return;
	}
}

void waterUpdateAll(){
	static uint calls = 0;
	for(uint i=(calls&0xF);i<waterCount;i+=0x10){
		waterUpdate(&waterList[i]);
	}
	calls++;
}

void waterSyncPlayer(uint c){
	if(waterCount == 0){return;}

	const int count = 8;
	for(;clients[c].waterUpdateOffset<MIN(waterCount,clients[c].waterUpdateOffset+count);clients[c].waterUpdateOffset++){
		waterSendUpdate(c,clients[c].waterUpdateOffset);
	}
	if(clients[c].waterUpdateOffset >= waterCount){clients[c].waterUpdateOffset = 0;}
}
