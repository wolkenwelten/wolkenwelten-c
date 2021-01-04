#include "landscape.h"

#include "../game/water.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/profiling.h"

#include <string.h>

static void landscapeUpdateChunk(chunk *c){
	static water *waters[16][16][16];
	static u8 borderBlocks[18][19][18];
	memset(waters,0,sizeof(waters));
	for(beingListEntry *ble = c->bl.first; ble != NULL; ble = ble->next){
		for(uint i=0;i<countof(ble->v);i++){
			const being b = ble->v[i];
			if(beingType(b) != BEING_WATER){continue;}
			const uint ID = beingID(b);
			if(ID >= waterCount){continue;}
			water *w = &waterList[beingID(b)];
			waters[w->x&0xF][w->y&0xF][w->z&0xF] = w;
		}
	}

	for(int x=-1; x < 17; x++){
	for(int y=-1; y < 18; y++){
	for(int z=-1; z < 17; z++){
		if((x|y|z) & (~15)){
			borderBlocks[x+1][y+1][z+1] = worldTryB(x+c->x,y+c->y,z+c->z);
		}else{
			borderBlocks[x+1][y+1][z+1] = c->data[x][y][z];
		}
	}
	}
	}

	for(uint x=0; x < 16; x++){
	for(uint y=0; y < 16; y++){
	for(uint z=0; z < 16; z++){
		const u8 b = c->data[x][y][z];
		water *w = waters[x][y][z];
		switch(b){
		default:
			break;
		case I_Dirt:
			if((w != NULL) && (w->amount > 128) && (y < 15) && (c->data[x][y+1][z] == 0) && (rngValA((1<<12)-1) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dry_Grass);
				w->amount -= 128;
			}
			break;
		case I_Grass:
			if((w == NULL) && (rngValA((1<<18)-1) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dry_Grass);
			}else if((w != NULL) && (w->amount > 16)){
				int cx = x + (rngValA(1)<<1);
				int cy = y + (rngValA(1)<<1);
				int cz = z + (rngValA(1)<<1);
				if((borderBlocks[cx][cy][cz] == I_Dirt) && (borderBlocks[cx][cy+1][cz] == 0)){
					worldSetB(c->x+cx-1,c->y+cy-1,c->z+cz-1,I_Dry_Grass);
					w->amount -= 16;
				}
			}
			break;
		case I_Dry_Grass:
			if((w != NULL) && (w->amount > 128)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Grass);
				w->amount -= 128;
			}
			break;
		}
	}
	}
	}
}

void landscapeUpdateAll(){
	static uint calls = 0;
	PROFILE_START();

	for(uint i=calls&0x3FF;i<chunkCount;i+=0x400){
		landscapeUpdateChunk(&chunkList[i]);
	}
	calls++;

	PROFILE_STOP();
}
