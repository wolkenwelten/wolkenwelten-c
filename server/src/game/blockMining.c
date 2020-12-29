#include "blockMining.h"

#include "../game/character.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdlib.h>

typedef struct {
	u16 x,y,z;
	i16 damage;
	u8  b;
	u8 wasMined;
} blockMining;

blockMining blockMiningList[64];
uint        blockMiningCount = 0;

int blockMiningNew(int x,int y,int z){
	blockMining *bm;
	if(!inWorld(x,y,z)){return -1;}
	if(blockMiningCount >= countof(blockMiningList)){return -1;}
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

void blockMiningMine(uint i, int dmg){
	blockMining *bm = &blockMiningList[i];

	bm->damage  += dmg;
	bm->wasMined = true;
}

int blockMiningMinePos(int dmg, int x, int y, int z){
	if(!inWorld(x,y,z)){return 1;}
	for(uint i=0;i<blockMiningCount;i++){
		blockMining *bm = &blockMiningList[i];
		if(bm->x != x){continue;}
		if(bm->y != y){continue;}
		if(bm->z != z){continue;}
		blockMiningMine(i,dmg);
		return 0;
	}
	if(worldGetB(x,y,z) == 0){return 1;}
	int i = blockMiningNew(x,y,z);
	if(i >= 0){
		blockMiningMine(i,dmg);
	}
	return 0;
}

int blockMiningMinePosItem(item *itm, int x, int y, int z){
	const u8 b = worldGetB(x,y,z);
	if(b == 0){return 1;}
	int dmg = 1;
	if(itm != NULL){
		dmg = blockDamageDispatch(itm,blockTypeGetCat(b));
	}
	return blockMiningMinePos(dmg,x,y,z);
}

void blockMiningMineBlock(int x, int y, int z, u8 b){
	msgMineBlock(x,y,z,b);
	if((b == I_Grass) || (b == I_Dry_Grass) || (b == I_Roots)){
		worldSetB(x,y,z,I_Dirt);
	}else{
		worldSetB(x,y,z,0);
		blockMiningDropItemsPos(x,y,z,b);
	}
}

void blockMiningUpdate(){
	for(uint i=0;i<clientCount;++i){
		if(clients[i].c == NULL)          {continue;}
		if(clients[i].c->blockMiningX < 0){continue;}
		item *itm = characterGetItemBarSlot(clients[i].c,clients[i].c->activeItem);
		if(blockMiningMinePosItem(itm,clients[i].c->blockMiningX,clients[i].c->blockMiningY,clients[i].c->blockMiningZ)){
			clients[i].c->blockMiningX = clients[i].c->blockMiningY = clients[i].c->blockMiningZ = -1;
		}
	}

	for(uint i=blockMiningCount-1;i<blockMiningCount;i--){
		blockMining *bm = &blockMiningList[i];
		if(!bm->wasMined){
			const int maxhp = blockTypeGetHP(bm->b);
			bm->damage -= maxhp >> 5;
			if(bm->damage <= 0){
				blockMiningList[i] = blockMiningList[--blockMiningCount];
			}
		}else{
			if(bm->damage > blockTypeGetHP(bm->b)){
				blockMiningMineBlock(bm->x,bm->y,bm->z,bm->b);
				blockMiningList[i] = blockMiningList[--blockMiningCount];
			}
			bm->wasMined = false;
		}
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

uint blockMiningGetActive(){
	return blockMiningCount;
}
