#include "fire.h"

#include "../game/blockMining.h"
#include "../game/grenade.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>

void fireNewF(u16 x, u16 y, u16 z, i16 strength, i16 blockDmg){
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
	fireSendUpdate(-1,(int)(f - fireList));
}

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

void fireUpdate(fire *f){
	if(f == NULL){return;}
	f->strength -= 2;
	if(f->strength <= 0){
		if(isClient){return;}
		fireList[f-fireList] = fireList[--fireCount];
		fireSendUpdate(-1,f-fireList);
		return;
	}
	const u8 b = worldGetB(f->x,f->y,f->z);
	const int dmg = blockTypeGetFireDmg(b);

	f->strength = MIN(30000,f->strength+dmg-1);
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
	if(f->strength > 8192){
		f->strength -= 1024;
		explode(vecAdd(vecNew(f->x,f->y,f->z),vecMulS(vecRng(),4.f)), 0.5f, 0);
	}else if(f->strength > 2048){
		f->strength -= 512;
		explode(vecAdd(vecNew(f->x,f->y,f->z),vecMulS(vecRng(),3.f)), 0.3f, 0);
	}

	fireSpread(f);
}

void fireUpdateAll(){
	static uint calls = 0;
	for(uint i=(calls&0x1F);i<fireCount;i+=0x20){
		fireUpdate(&fireList[i]);
	}
	calls++;
}

void fireSyncPlayer(uint c){
	if(fireCount == 0){return;}

	const int count = 8;
	for(;clients[c].fireUpdateOffset<MIN(fireCount,clients[c].fireUpdateOffset+count);clients[c].fireUpdateOffset++){
		fireSendUpdate(c,clients[c].fireUpdateOffset);
	}
	if(clients[c].fireUpdateOffset >= fireCount){clients[c].fireUpdateOffset = 0;}
}
