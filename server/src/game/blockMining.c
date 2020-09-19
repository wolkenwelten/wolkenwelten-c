#include "blockMining.h"

#include "../../../common/src/game/blockType.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
	int x,y,z;
	int damage;
	u8  b;
	int wasMined;
} blockMining;

blockMining blockMiningList[128];
uint        blockMiningCount = 0;

int blockMiningNew(int x,int y,int z){
	blockMining *bm;
	bm = &blockMiningList[blockMiningCount++];
	bm->x = x;
	bm->y = y;
	bm->z = z;
	bm->damage = 0;
	bm->wasMined = false;
	bm->b = worldGetB(x,y,z);
	return blockMiningCount-1;
}

float blockMiningGetProgress(blockMining *bm){
	if(bm->b == 0){return 0.f;}
	return ((float)bm->damage) / ((float)blockTypeGetHP(bm->b));
}

void blockMiningDropItemsPos(int x, int y, int z, u8 b){
	u16 ID = b;
	if(b == 0){return;}
	if(b == 7){return;}  // Roots
	if(b == 2){ID = 1;}  // Grass
	if(b == 8){ID = 1;}  // Roots
	if((b == 19) || (b == 21)){
		ID = 272;
		if(rngValM(8)!=0){
			return;
		}
	}
	if((b == 6) || (b == 11)){ // Leaves
		ID = 258;
		if(rngValM(8)!=0){
			return;
		}
	}
	item i = itemNew(ID,1);
	float xoff = ((float)rngValM(1024) / 2048.f)+0.25f;
	float yoff = ((float)rngValM(1024) / 4096.f)+0.25f;
	float zoff = ((float)rngValM(1024) / 2048.f)+0.25f;
	itemDropNewP(vecNew(x + xoff,y + yoff,z + zoff), &i);
}

void blockMiningDropItems(blockMining *bm){
	blockMiningDropItemsPos(bm->x,bm->y,bm->z,bm->b);
}

void blockMiningMine(uint i, item *itm){
	blockMining *bm = &blockMiningList[i];

	bm->damage += blockDamageDispatch(itm,blockTypeGetCat(bm->b));
	bm->wasMined = true;
	if(bm->damage > blockTypeGetHP(bm->b)){
		msgMineBlock(bm->x,bm->y,bm->z,bm->b);
		worldSetB(bm->x,bm->y,bm->z,0);
		blockMiningDropItems(bm);
		blockMiningList[i] = blockMiningList[--blockMiningCount];
	}
}

void blockMiningMinePos(item *itm, int x, int y, int z){
	for(uint i=0;i<blockMiningCount;i++){
		blockMining *bm = &blockMiningList[i];
		if(bm->x != x){continue;}
		if(bm->y != y){continue;}
		if(bm->z != z){continue;}
		blockMiningMine(i,itm);
		return;
	}
	int i = blockMiningNew(x,y,z);
	if(i >= 0){
		blockMiningMine(i,itm);
	}
}

void blockMiningUpdate(){
	for(uint i=0;i<clientCount;++i){
		if(clients[i].c == NULL)          {continue;}
		if(clients[i].c->blockMiningX < 0){continue;}
		item *itm = characterGetItemBarSlot(clients[i].c,clients[i].c->activeItem);
		blockMiningMinePos(itm,clients[i].c->blockMiningX,clients[i].c->blockMiningY,clients[i].c->blockMiningZ);
	}

	for(uint i=0;i<blockMiningCount;++i){
		blockMining *bm = &blockMiningList[i];
		if(!bm->wasMined){
			bm->damage -= blockTypeGetHP(bm->b) >> 5;
			if(bm->damage <= 0){
				blockMiningList[i] = blockMiningList[--blockMiningCount];
				--i;
				continue;
			}
		}
		bm->wasMined = false;
	}
}

void blockMiningUpdatePlayer(uint c){
	if(blockMiningCount == 0){
		msgBlockMiningUpdate(c,0,0,0,0,0,10);
		return;
	}
	for(uint i=0;i<blockMiningCount;++i){
		const blockMining *bm = &blockMiningList[i];
		msgBlockMiningUpdate(c,bm->x,bm->y,bm->z,bm->damage,blockMiningCount,i);
	}
}
