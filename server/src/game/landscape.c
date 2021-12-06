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

#include "weather/weather.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/profiling.h"

#include <stdio.h>
#include <string.h>

static bool shouldThaw(){
	return ((int)rngValA(255) > (snowIntensity << 4));
}

static void landscapeUpdateChunk(chunk *c){
	static blockId topBlocks[CHUNK_SIZE][CHUNK_SIZE];

	chunk *tc = worldTryChunk(c->x,c->y+CHUNK_SIZE,c->z);
	if(tc == NULL){
		memset(topBlocks,0,sizeof(topBlocks));
	}else{
		for(int x=0;x<CHUNK_SIZE;x++){
		for(int z=0;z<CHUNK_SIZE;z++){
			topBlocks[x][z] = tc->data[x][0][z];
		}
		}
	}

	for(uint x=0; x < CHUNK_SIZE; x++){
	for(uint y=0; y < CHUNK_SIZE; y++){
	for(uint z=0; z < CHUNK_SIZE; z++){
		const blockId b = c->data[x][y][z];
		const blockId tb = y == CHUNK_SIZE-1 ? topBlocks[x][z] : c->data[x][y+1][z];

		switch(b){
		default:
			break;
		case I_Snow_Dirt:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dirt);
			}
			break;
		case I_Snow_Grass:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Grass);
			}
			break;
		case I_Snowy_Spruce_Leaf:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Spruce_Leaf);
			}
			break;
		case I_Snowy_Oak_Leaf:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Oak_Leaf);
			}
			break;
		case I_Snowy_Flower:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Flower);
			}
			break;
		case I_Snowy_Date:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Date);
			}
			break;
		case I_Snowy_Acacia_Leaf:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Acacia_Leaf);
			}
			break;
		case I_Snowy_Roots:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Roots);
			}
			break;
		case I_Snowy_Sakura_Leaf:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Sakura_Leaf);
			}
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
