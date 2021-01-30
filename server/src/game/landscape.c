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

#include "landscape.h"

#include "../game/weather.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/profiling.h"

#include <string.h>

static void landscapeUpdateChunk(chunk *c){
	static u8 topBlocks[16][16];

	chunk *tc = worldTryChunk(c->x,c->y+1,c->z);
	if(tc == NULL){
		memset(topBlocks,0,sizeof(topBlocks));
	}else{
		for(int x=0;x<16;x++){
		for(int z=0;z<16;z++){
			topBlocks[x][z] = tc->data[x][15][z];
		}
		}
	}

	for(uint x=0; x < 16; x++){
	for(uint y=0; y < 16; y++){
	for(uint z=0; z < 16; z++){
		const u8 b = c->data[x][y][z];
		u8 tb;
		if(y == 15){
			tb = topBlocks[x][z];
		}else{
			tb = c->data[x][y+1][z];
		}

		switch(b){
		default:
			break;
		case I_Dry_Grass:
		case I_Grass:
			if((tb != 0) && (rngValA(15) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dirt);
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

	for(uint i=calls&0x3FFF;i<chunkCount;i+=0x4000){
		landscapeUpdateChunk(&chunkList[i]);
	}
	calls++;

	PROFILE_STOP();
}
