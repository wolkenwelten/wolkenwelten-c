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

void chunkOptimizePlane(u32 plane[CHUNK_SIZE][CHUNK_SIZE]){
	for(int y=CHUNK_SIZE-1;y>=0;y--){
	for(int x=CHUNK_SIZE-1;x>=0;x--){
		if(!plane[x][y]){continue;}
		if((x < CHUNK_SIZE-2) && ((plane[x][y] & 0xFFFF00FF) == (plane[x+1][y] & 0xFFFF00FF))){
			plane[x  ][y] += plane[x+1][y] & 0xFF00;
			plane[x+1][y]  = 0;
		}
		if((y < CHUNK_SIZE-2) && ((plane[x][y] & 0xFF00FFFF) == (plane[x][y+1] & 0xFF00FFFF))){
			plane[x][y  ] += plane[x][y+1]&0xFF0000;
			plane[x][y+1]  = 0;
		}
	}
	}
}
