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
#include <stdio.h>

void fluidPhysics(chunkOverlay *fluid, int cx, int cy, int cz){
	(void)cx;
	(void)cy;
	(void)cz;
	for(int x=0;x<CHUNK_SIZE;x++){
	for(int z=0;z<CHUNK_SIZE;z++){
		int curLevel = fluid->data[x][0][z] & 0xF8;
		for(int y=CHUNK_SIZE-1;y>=0;y--){
			const int lastLevel = curLevel;
			curLevel = fluid->data[x][y][z];
			if(curLevel == 0){continue;}
			if((curLevel & 0x7) != (lastLevel & 0x7)){continue;}
			const int flowAmount = MIN((0xF8 - (lastLevel & 0xF8)),(curLevel & 0xF8));
			if(flowAmount){
				printf("a: %x   b: %x  flow:%x\n", curLevel, lastLevel, flowAmount);
				fluid->data[x][y-1][z] = lastLevel + flowAmount;
				fluid->data[x][y][z] = curLevel = curLevel - flowAmount;
			}
		}
	}
	}
}
