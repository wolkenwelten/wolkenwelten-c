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
#include "shared.h"

#include "../chunk.h"

int chunksGeneratedThisFrame = 0;

uint chunkGetGeneratedThisFrame(){
	return chunksGeneratedThisFrame;
}

void chunkResetCounter(){
	chunksGeneratedThisFrame = 0;
}

sideMask chunkGetSides(u16 x,u16 y,u16 z,blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2]){
	sideMask sides = 0;

	if(b[x][y][z+1] == 0){ sides |= sideMaskFront; }
	if(b[x][y][z-1] == 0){ sides |= sideMaskBack;  }
	if(b[x][y+1][z] == 0){ sides |= sideMaskTop;   }
	if(b[x][y-1][z] == 0){ sides |= sideMaskBottom;}
	if(b[x+1][y][z] == 0){ sides |= sideMaskLeft;  }
	if(b[x-1][y][z] == 0){ sides |= sideMaskRight; }

	return sides;
}

void chunkOptimizePlane(u64 plane[CHUNK_SIZE][CHUNK_SIZE]){
	for(int y=CHUNK_SIZE-1;y>=0;y--){
	for(int x=CHUNK_SIZE-1;x>=0;x--){
		if(!plane[x][y]){continue;}
		if((x < CHUNK_SIZE-2) && ((plane[x][y] & 0xFFFF0FF) == (plane[x+1][y] & 0xFFFF0FF))){
			plane[x  ][y] += plane[x+1][y] & 0xF00;
			plane[x+1][y]  = 0;
		}
		if((y < CHUNK_SIZE-2) && ((plane[x][y] & 0xFFF0FFF) == (plane[x][y+1] & 0xFFF0FFF))){
			plane[x][y  ] += plane[x][y+1]&0xF000;
			plane[x][y+1]  = 0;
		}
	}
	}
}

void chunkPopulateBlockData(blockId b[CHUNK_SIZE+2][CHUNK_SIZE+2][CHUNK_SIZE+2], chunk *c, i16 xoff, i16 yoff, i16 zoff){
	if(c->block == NULL){return;}
	for(int x=MAX(0,xoff); x<MIN(CHUNK_SIZE+2,xoff+CHUNK_SIZE); x++){
	for(int y=MAX(0,yoff); y<MIN(CHUNK_SIZE+2,yoff+CHUNK_SIZE); y++){
	for(int z=MAX(0,zoff); z<MIN(CHUNK_SIZE+2,zoff+CHUNK_SIZE); z++){
		b[x][y][z] = c->block->data[x-xoff][y-yoff][z-zoff];
	}
	}
	}
}
