#include "fire.h"

#include "../game/blockMining.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>

void fireNew(u16 x, u16 y, u16 z, i16 strength){
	fire *f = NULL;

	for(uint i=fireCount-1;i<fireCount;i--){
		if(fireList[i].x != x){continue;}
		if(fireList[i].y != y){continue;}
		if(fireList[i].z != z){continue;}
		f = &fireList[i];
		break;
	}
	if(f == NULL){
		if(fireCount < countof(fireList)){
			f = &fireList[fireCount++];
		}else{
			f = &fireList[rngValM(countof(fireList))];
		}
		f->strength = 0;
		f->blockDmg = 0;
	}

	f->x = x;
	f->y = y;
	f->z = z;
	f->strength += strength;
	fireSendUpdate(-1,(int)(f - fireList));
}

void fireIntro(uint c){
	for(uint i=0;i<fireCount;i++){
		fireSendUpdate(c,i);
	}
}

void fireRecvUpdate(uint c, const packet *p){
	(void)c;
	const u16 x        = p->v.u16[2];
	const u16 y        = p->v.u16[3];
	const u16 z        = p->v.u16[4];
	const i16 strength = p->v.i16[5];
	fireNew(x,y,z,strength);
}

fire *fireGetAtPos(u16 x,u16 y, u16 z){
	for(uint i=0;i<fireCount;i++){
		fire *f = &fireList[i];
		if(f->x != x){continue;}
		if(f->y != y){continue;}
		if(f->z != z){continue;}
		return f;
	}
	return NULL;
}

static inline void fireSpreadToBlock(fire *f, int r){
	u16 fx,fy,fz;
	fx = (f->x - r)+rngValM(r*2+1);
	fy = (f->y - r)+rngValM(r*2+1);
	fz = (f->z - r)+rngValM(r*2+1);

	if((fy < f->y) && (rngValR() & 1)){
		fy = fy + (f->y-fy);
	}
	const u8 nb = worldGetB(fx,fy,fz);
	if(nb == 0){return;}
	fireNew(fx,fy,fz,8);
	f->strength -= 8;
}

static inline void fireSpreadToLeaf(fire *f, int r){
	u16 fx,fy,fz;
	fx = (f->x - r)+rngValM(r*2+1);
	fy = (f->y - r)+rngValM(r*2+1);
	fz = (f->z - r)+rngValM(r*2+1);

	if((fy < f->y) && (rngValR() & 1)){
		fy = fy + (f->y-fy);
	}
	const u8 nb = worldGetB(fx,fy,fz);
	blockCategory nbt = blockTypeGetCat(nb);
	if(nbt != LEAVES){return;}
	fireNew(fx,fy,fz,8);
	f->strength -= 8;
}

static inline void fireSpread(fire *f){
	int count = f->strength >> 7;
	for(int i=0;i<count;i++){
		if((rngValR() & 0x1F)!= 0){continue;}
		int r = 1;
		if((f->strength > 0xFF) && ((rngValR() & 0x07) == 0)){r = 2;}
		fireSpreadToBlock(f,r);
	}
	count = f->strength >> 6;
	for(int i=0;i<count;i++){
		if((rngValR() & 0x1F)!= 0){continue;}
		int r = 2;
		if((f->strength > 0xFF) && ((rngValR() & 0x07) == 0)){r = 3;}
		fireSpreadToLeaf(f,r);
	}
}

static int fireUpdate(fire *f){
	f->strength -= 2;
	if(f->strength <= 0){
		return 1;
	}
	const u8 b = worldGetB(f->x,f->y,f->z);
	const int dmg = blockTypeGetFireDmg(b);

	f->strength += dmg-1;
	if(b == 0){
		f->blockDmg = 0;
	}else{
		const int maxhp = blockTypeGetHP(b);
		f->blockDmg = MIN(maxhp,f->blockDmg + 1);
		if(f->blockDmg >= maxhp){
			blockMiningMineBlock(f->x,f->y,f->z,b);
			f->blockDmg = 0;
		}
	}

	fireSpread(f);
	return 0;
}

void fireUpdateAll(){
	for(uint i=0;i<fireCount;i++){
		if (fireUpdate(&fireList[i])){
			if(!isClient){
				fireList[i] = fireList[--fireCount];
				fireSendUpdate(-1,i);
			}
		}
	}
}

void fireSyncPlayer(uint c){
	if(fireCount == 0){return;}

	const int count = 8;
	for(;clients[c].fireUpdateOffset<MIN(fireCount,clients[c].fireUpdateOffset+count);clients[c].fireUpdateOffset++){
		fireSendUpdate(c,clients[c].fireUpdateOffset);
	}
	if(clients[c].fireUpdateOffset >= fireCount){clients[c].fireUpdateOffset = 0;}
}
