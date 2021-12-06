/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "blockMining.h"

#include "../game/character.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
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
	return ((float)bm->damage) / ((float)blockTypeGetHealth(bm->b));
}

void blockMiningDropItemsPos(int x, int y, int z, blockId b){
	u16 ID = b;
	if(b == 0)          {return;}
	if(b == I_Roots)    {return;}  // Roots
	if(b == I_Grass)    {ID = 1;}  // Grass
	if(b == I_Dry_Grass){ID = 1;}  // Grass
	if((b == I_Sakura_Leaf) || (b == I_Flower) || (b == I_Date)){
		ID = 272;
		if(rngValM(8)!=0){
			return;
		}
	}
	if((b == I_Oak_Leaf) || (b == I_Spruce_Leaf) || (b == I_Acacia_Leaf)){ // Leaves
		ID = 258;
		if(rngValM(8)!=0){
			return;
		}
	}
	item i = itemNew(ID,1);
	float xoff = ((float)rngValM(1024) / 2048.f)+0.25f;
	float yoff = ((float)rngValM(1024) / 4096.f)+0.25f;
	float zoff = ((float)rngValM(1024) / 2048.f)+0.25f;
	itemDropNewP(vecNew(x + xoff,y + yoff,z + zoff), &i, -1);
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
	const blockId b = worldGetB(x,y,z);
	if(b == 0){return 1;}
	int dmg = 1;
	if(itm != NULL){
		dmg = itemGetDamage(itm,blockTypeGetCat(b));
	}
	return blockMiningMinePos(dmg,x,y,z);
}

void blockMiningMineBlock(int x, int y, int z, u8 cause){
	const blockId b = worldGetB(x,y,z);
	if(b == 0){return;}
	msgMineBlock(x,y,z,b,0);
	if((b == I_Grass) || (b == I_Dry_Grass) || (b == I_Roots) || (b == I_Snow_Grass)){
		worldSetB(x,y,z,I_Dirt);
		if(cause == 0){
			switch(b){
			case I_Snow_Grass: {
				item itm = itemNew(I_Plantmatter,1);
				itemDropNewP(vecNew(x,y,z),&itm,-1);
				break; }
			case I_Roots: {
				item itm = itemNew(I_Oak,1);
				itemDropNewP(vecNew(x,y,z),&itm,-1);
				break; }
			case I_Grass: {
				item itm = itemNew(I_Plantmatter,1);
				itemDropNewP(vecNew(x,y,z),&itm,-1);
				break; }
			case I_Dry_Grass: {
				item itm = itemNew(I_Straw,1);
				itemDropNewP(vecNew(x,y,z),&itm,-1);
				break; }
			}
		}
	}else if(b == I_Snow_Dirt){
		worldSetB(x,y,z,0);
		blockMiningDropItemsPos(x,y,z,I_Dirt);
	}else{
		worldSetB(x,y,z,0);
		blockMiningDropItemsPos(x,y,z,b);
	}
}

void blockMiningBurnBlock(int x, int y, int z, blockId b){
	msgMineBlock(x,y,z,b,1);
	if((b == I_Grass) || (b == I_Dry_Grass) || (b == I_Roots)){
		worldSetB(x,y,z,I_Dirt);
	}else{
		worldSetB(x,y,z,0);
	}
}

static void blockMiningDel(int i){
	blockMiningList[i] = blockMiningList[--blockMiningCount];
	if(blockMiningCount == 0){msgBlockMiningUpdate(-1,0,0,0,0,0,10);}
}

void blockMiningUpdateAll(){
	PROFILE_START();

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
			const int maxhp = blockTypeGetHealth(bm->b);
			bm->damage -= maxhp >> 5;
			if(bm->damage <= 0){blockMiningDel(i);}
		}else{
			bm->wasMined = false;
			if(bm->damage > blockTypeGetHealth(bm->b)){
				blockMiningMineBlock(bm->x,bm->y,bm->z,0);
				blockMiningDel(i);
			}
		}
	}

	PROFILE_STOP();
}

void blockMiningUpdatePlayer(uint c){
	if(blockMiningCount == 0){
		if((clients[c].syncCount & 0xFF) != msgtBlockMiningUpdate){return;}
		msgBlockMiningUpdate(c,0,0,0,0,0,10);
	}else{
		for(uint i=0;i<blockMiningCount;++i){
			const blockMining *bm = &blockMiningList[i];
			msgBlockMiningUpdate(c,bm->x,bm->y,bm->z,bm->damage,blockMiningCount,i);
		}
	}
}

uint blockMiningGetActive(){
	return blockMiningCount;
}
