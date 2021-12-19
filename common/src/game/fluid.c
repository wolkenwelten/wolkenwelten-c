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
#include "fluid.h"
#include "../world/world.h"

int fluidPhysics(chunkOverlay *fluid, int cx, int cy, int cz){
	(void)cx;
	(void)cy;
	(void)cz;
	int accumulator = 0;
	for(int x=0;x<CHUNK_SIZE;x++){
	for(int z=0;z<CHUNK_SIZE;z++){
		int curLevel = worldGetFluid(cx+x,cy-1,cz+z);
		u8 curBlock = worldGetB(cx+x,cy-1,cz+z);
		accumulator += curLevel;
		for(int y=0;y<CHUNK_SIZE;y++){
			const int botLevel = curLevel;
			const u8 botBlock = curBlock;
			curLevel = fluid->data[x][y][z];
			curBlock = worldGetB(cx+x,cy+y,cz+z);
			if(curLevel == 0){continue;}
			if(botBlock){continue;}
			const u8 botType = (botLevel & 0x7);
			if(botType && (botType != (curLevel & 0x7))){continue;}
			const int flowAmount = MIN((0xF8 - (botLevel & 0xF8)),(curLevel & 0xF8));
			if(flowAmount){
				curLevel = fluid->data[x][y][z] = curLevel - flowAmount;
				if(curLevel < 8){curLevel = fluid->data[x][y][z] = 0;}
				if(y == 0){
					worldSetFluid(cx+x,cy-1,cz+z,botLevel+flowAmount);
				}else{
					fluid->data[x][y-1][z] = botLevel + flowAmount;
				}
			}
		}
	}
	}
	return accumulator;
}
