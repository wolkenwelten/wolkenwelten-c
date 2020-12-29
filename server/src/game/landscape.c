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

	for(uint x=0;x<16;x++){
	for(uint y=0;y<16;y++){
	for(uint z=0;z<16;z++){
		const u8 b = c->data[x][y][z];
		water *w = waters[x][y][z];
		switch(b){
		default:
			break;
		case I_Grass:
			if((w == NULL) && (rngValA((1<<14)-1) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dry_Grass);
			}
			break;
		case I_Dry_Grass:
			if((w != NULL) && (rngValA((1<<6)-1) == 0)){
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
	for(uint i=calls&0x1F;i<chunkCount;i+=0x20){
		landscapeUpdateChunk(&chunkList[i]);
	}
	calls++;
}
