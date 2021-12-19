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

static int fluidFlowInto(int curLevel, int toX, int toY, int toZ, int maxFlow){
	const int toLevel = worldGetFluid(toX,toY,toZ);
	const u8 toBlock = worldTryB(toX, toY, toZ);
	if(toBlock){return curLevel;}
	const u8 toType = toLevel & 0x7;
	if(toType && (toType != (curLevel & 0x7))){return curLevel;}
	const int flowAmount = MIN(maxFlow,MIN((0xF8 - (toLevel & 0xF8)),(curLevel & 0xF8)));
	worldSetFluid(toX, toY, toZ, toLevel + flowAmount);
	curLevel -= flowAmount;
	return curLevel < 8 ? 0 : curLevel;
}

int fluidPhysics(chunkOverlay *fluid, int cx, int cy, int cz){
	for(int x=0;x<CHUNK_SIZE;x++){
	for(int z=0;z<CHUNK_SIZE;z++){
	for(int y=0;y<CHUNK_SIZE;y++){
		int curLevel = fluid->data[x][y][z];
		if(curLevel){
			curLevel = fluidFlowInto(curLevel, cx+x, cy+y-1, cz+z, curLevel & 0xF8);
			if(curLevel){
				const int sideFlow = MAX(((curLevel & 0xF8) / 4),4);
				curLevel = fluidFlowInto(curLevel, cx+x-1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowInto(curLevel, cx+x+1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowInto(curLevel, cx+x, cy+y, cz+z-1, sideFlow);
				curLevel = fluidFlowInto(curLevel, cx+x, cy+y, cz+z+1, sideFlow);
			}
			if(((curLevel & 0xF8) == 0x8) && (rngValA(0xFF) == 0)){curLevel = 0;} // Evaporation
			fluid->data[x][y][z] = curLevel;
		}
	}
	}
	}
	return 0;
}
