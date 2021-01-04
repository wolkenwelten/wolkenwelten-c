#include "fire.h"

#include "../game/being.h"
#include "../game/blockMining.h"
#include "../game/grenade.h"
#include "../game/water.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>

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

static inline void fireSpreadToBlock(fire *f, int r){
	u16 fx,fy,fz;
	fx = (f->x - r) + rngValM(r*2+1);
	fy = (f->y - r) + rngValM(r*2+1);
	fz = (f->z - r) + rngValM(r*2+1);

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
	fx = (f->x - r) + rngValM(r*2+1);
	fy = (f->y - r) + rngValM(r*2+1);
	fz = (f->z - r) + rngValM(r*2+1);

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
	for(int i=0;i<count+1;i++){
		if((rngValR() & 0x1F)!= 0){continue;}
		int r = 1;
		const uint rv = (rngValR() & 0x07);
		if(     (f->strength > 0x1F) && (rv != 0)){r = 2;}
		else if((f->strength > 0xFF) && (rv == 0)){r = 3;}
		fireSpreadToLeaf(f,r);
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
	const int dmg = MIN(f->oxygen,blockTypeGetFireDmg(b));

	f->strength = MIN(30000,f->strength+dmg-1);
	f->oxygen  -= dmg;
	f->oxygen  += MIN(airB,64-f->oxygen);

	water *w = waterGetAtPos(f->x,f->y,f->z);
	if((w != NULL) && (w->amount > 0)){
		f->oxygen    = 0;
		f->strength -= w->amount;
		w->amount   -= f->strength;
	}
	if(f->strength <= 0){
		fireDel(f-fireList);
		fireSendUpdate(-1,fireCount);
		fireSendUpdate(-1,f-fireList);
		return;
	}
	int wx = f->x-1+rngValM(3);
	int wy = f->y-1+rngValM(3);
	int wz = f->z-1+rngValM(3);
	w = waterGetAtPos(wx,wy,wz);
	if((w != NULL) && (w->amount > 0)){
		f->oxygen    = 0;
		f->strength -= w->amount;
		w->amount   -= f->strength;
	}
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
		const int maxhp = blockTypeGetFireHP(b);
		f->blockDmg = MIN(maxhp,f->blockDmg + dmg);
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

void fireDelChungus(const chungus *c){
	if(c == NULL){return;}
	const u16 cx = c->x << 8;
	const u16 cy = c->y << 8;
	const u16 cz = c->z << 8;
	for(uint i=fireCount-1;i<fireCount;i--){
		const fire *f = &fireList[i];
		if((f->x & 0xFF00) != cx){continue;}
		if((f->y & 0xFF00) != cy){continue;}
		if((f->z & 0xFF00) != cz){continue;}
		fireDel(i);
	}
}
