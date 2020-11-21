#include "fire.h"

#include "../game/blockMining.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"


void fireNew(const vec pos, u16 strength){
	uint i;
	if(fireCount < countof(fireList)){
		i = fireCount++;
	}else{
		i = rngValM(countof(fireList));
	}
	fire *f = &fireList[i];
	f->x = pos.x;
	f->y = pos.y;
	f->z = pos.z;
	f->strength = strength;
	fireSendUpdate(-1,i);
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
	fireNew(vecNew(x,y,z),strength);
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

static int fireUpdate(fire *f){
	f->strength -= 2;
	if(f->strength <= 0){
		return 1;
	}
	const u8 b = worldGetB(f->x,f->y,f->z);
	const blockCategory cat = blockTypeGetCat(b);
	int dmg = 1;
	switch(cat){
	default:
		break;
	case WOOD:
		dmg = 3;
		break;
	case LEAVES:
		dmg = 5;
		break;
	}
	switch(b){
	case I_Grass:
	case I_Coal:
		dmg = 3;
		break;
	default:
		break;
	}

	f->strength += dmg;
	blockMiningMinePos(dmg,f->x,f->y,f->z);

	if((rngValR() & 0xF) != 0){return 0;}
	u16 fx,fy,fz;
	if((rngValR() & 0x7) != 0){
		fx = (f->x - 1)+rngValM(3);
		fy = (f->y - 1)+rngValM(3);
		fz = (f->z - 1)+rngValM(3);
	}else{
		fx = (f->x - 2)+rngValM(5);
		fy = (f->y - 2)+rngValM(5);
		fz = (f->z - 2)+rngValM(5);
	}
	if((fy < f->y) && (rngValR() & 1)){
		fy = fy + (f->y-fy);
	}

	fire *nf = fireGetAtPos(fx,fy,fz);
	if(nf == NULL){
		const u8 nb = worldGetB(fx,fy,fz);
		if(nb == 0){return 0;}
		fireNew(vecNew(fx,fy,fz),16);
		f->strength -= 16;
	}else{
		nf->strength += 2;
		f->strength  -= 2;
	}
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
