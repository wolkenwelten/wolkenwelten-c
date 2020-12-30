#include "landscape.h"

#include "../game/water.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/item.h"

#include <string.h>

static void landscapeUpdateChunk(chunk *c){
	static water *waters[16][16][16];
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

	for(uint x=0; x < 16; x++){
	for(uint y=0; y < 16; y++){
	for(uint z=0; z < 16; z++){
		const u8 b = c->data[x][y][z];
		water *w = waters[x][y][z];
		switch(b){
		default:
			break;
		case I_Dirt:
			if((w != NULL) && (w->amount > 128) && (y < 15) && (c->data[x][y+1][z] == 0)){
				uint i = 20;
				u8 cx = (x-1) + (rngValA(1)<<1);
				u8 cy = (y-1) + (rngValA(1)<<1);
				u8 cz = (z-1) + (rngValA(1)<<1);
				if(c->data[cx][cy][cz] == I_Dry_Grass){ i = 16; }
				if(c->data[cx][cy][cz] == I_Grass)    { i = 12; }
				if(rngValA((1 << i)-1) == 0){
					worldSetB(c->x+x,c->y+y,c->z+z,I_Dry_Grass);
					w->amount -= 128;
				}
			}
			break;
		case I_Grass:
			if((w == NULL) && (rngValA((1<<14)-1) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dry_Grass);
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
	for(uint i=calls&0x3FF;i<chunkCount;i+=0x400){
		landscapeUpdateChunk(&chunkList[i]);
	}
	calls++;
}
