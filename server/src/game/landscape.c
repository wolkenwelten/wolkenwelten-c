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
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/profiling.h"

#include <stdio.h>
#include <string.h>

static bool shouldThaw(){
	return ((int)rngValA(255) > (snowIntensity << 4));
}

static void landscapeUpdateChunk(chunk *c, const chungus *chng){
	static blockId topBlocks[CHUNK_SIZE][CHUNK_SIZE];
	if((c == NULL) || (c->block == NULL)){return;}

	chunk *tc = worldTryChunk(c->x,c->y+CHUNK_SIZE,c->z);
	if((tc == NULL) || (tc->block == NULL)){
		memset(topBlocks,0,sizeof(topBlocks));
	}else{
		for(int x=0;x<CHUNK_SIZE;x++){
		for(int z=0;z<CHUNK_SIZE;z++){
			topBlocks[x][z] = tc->block->data[x][0][z];
		}
		}
	}

	for(uint x=0; x < CHUNK_SIZE; x++){
	for(uint y=0; y < CHUNK_SIZE; y++){
	for(uint z=0; z < CHUNK_SIZE; z++){
		const blockId b = c->block->data[x][y][z];
		const blockId tb = y == CHUNK_SIZE-1 ? topBlocks[x][z] : c->block->data[x][y+1][z];
		const u8 fluidLevel = c->fluid ? c->fluid->data[x][y][z] : 0;

		switch(b){
		default:
			break;
		case I_Snow_Dirt:
			if(shouldThaw()){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dirt);
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
		case I_Flower:
			if(chng->grassBlock != I_Grass){
				if(chng->grassBlock == I_Dry_Grass){
					if(!fluidLevel && (rngValA(15) == 0)){
						worldSetB(c->x+x,c->y+y,c->z+z,chng->grassBlock);
					}
				}
			}
			break;
		case I_Date:
			if(chng->grassBlock != I_Dry_Grass){
				if(chng->grassBlock == I_Grass){
					if(fluidLevel || (rngValA(15) == 0)){
						worldSetB(c->x+x,c->y+y,c->z+z,I_Flower);
					}
				}
			}else{
				if((fluidLevel) && (rngValA(15) == 0)){worldSetB(c->x+x,c->y+y,c->z+z,I_Flower);}
			}
			break;
		case I_Snow_Grass:
			if((tb != 0) && (rngValA(15) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dirt);
			}
			if(chng->grassBlock != I_Snow_Grass){
				if(rngValA(15) == 0){
					worldSetB(c->x+x,c->y+y,c->z+z,chng->grassBlock);
				}
			}
			break;
		case I_Dry_Grass:
			if((tb != 0) && (rngValA(15) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dirt);
			}
			if(chng->grassBlock != I_Dry_Grass){
				if(chng->grassBlock == I_Grass){
					if(fluidLevel || (rngValA(15) == 0)){
						worldSetB(c->x+x,c->y+y,c->z+z,chng->grassBlock);
					}
				}
			}else{
				if(fluidLevel && (rngValA(15) == 0)){
					worldSetB(c->x+x,c->y+y,c->z+z,I_Grass);
				}
			}
			break;
		case I_Grass:
			if((tb != 0) && (rngValA(15) == 0)){
				worldSetB(c->x+x,c->y+y,c->z+z,I_Dirt);
			}
			if(chng->grassBlock != I_Grass){
				if(chng->grassBlock == I_Dry_Grass){
					if(rngValA(15) == 0){
						if(fluidLevel){
							worldSetFluid(c->x+x,c->y+y,c->z+z,fluidLevel - 0x10);
						}else{
							worldSetB(c->x+x,c->y+y,c->z+z,chng->grassBlock);
						}
					}
				}
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

	const uint max = chungusGetLinearMax();
	for(uint i=calls&0x3FF;i<max;i+=0x400){
		const chungus *chng = chungusGetLinearChungus(i);
		landscapeUpdateChunk(chungusGetLinearChunk(i), chng);
	}
	calls++;

	PROFILE_STOP();
}
