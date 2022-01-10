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
#include "../../../common/src/game/blockType.h"
#include "../world/world.h"

#define EVAPORATION_CHANCE 0xFF

static int fluidFlowInto(u8 toBlock, u8 toLevel, int curLevel, int toX, int toY, int toZ){
	const uint ingressMask = blockTypeGetIngressMask(toBlock);
	if((toLevel & 0xF0) == 0xF0){return curLevel;}
	if((ingressMask == 0xFFFF) || (rngValA(ingressMask))){return curLevel;}
	worldSetFluid(toX, toY, toZ, toLevel + 0x10);
	curLevel -= 0x10;
	return curLevel < 0x10 ? 0 : curLevel;
}

static int fluidFlowOut(int curLevel, u8 curBlock, int toX, int toY, int toZ){
	const u8 toLevel = worldTryFluid(toX, toY, toZ);
	if((toLevel & 0xF0) == 0xF0){return curLevel;}
	const uint egressMask = blockTypeGetEgressMask(curBlock);
	if((egressMask == 0xFFFF) || (rngValA(egressMask))){return curLevel;}
	const u8 toBlock = worldTryB(toX, toY, toZ);
	if(toBlock){
		curLevel = fluidFlowInto(toBlock, toLevel, curLevel, toX, toY, toZ);
	}else{
		worldSetFluid(toX, toY, toZ, toLevel + 0x10);
		curLevel -= 0x10;
	}
	return curLevel < 0x10 ? 0 : curLevel;
}

static int fluidFlowHorizontaly(int curLevel, int toX, int toY, int toZ, int maxFlow){
	if(curLevel < 0x10){return curLevel;}
	const u8 toLevel = worldGetFluid(toX,toY,toZ);
	const u8 toType = toLevel & 0xF;
	const u8 toAmount = toLevel & 0xF0;
	if(toType && (toType != (curLevel & 0xF))){return curLevel;}
	if(toLevel > curLevel){return curLevel;}
	const u8 toBlock = worldTryB(toX, toY, toZ);
	if(toBlock){return fluidFlowInto(toBlock, toLevel, curLevel, toX, toY, toZ);}
	const int flowAmount = MIN(0xF0 - toAmount, maxFlow);
	const u8 toNew = toType | (0xF0 & (toAmount + flowAmount));
	worldSetFluid(toX, toY, toZ, toNew);
	const int actualFlow = (int)toNew - (int)toLevel;
	curLevel -= actualFlow;
	return curLevel < 0x10 ? 0 : curLevel;
}

static int fluidFlowDown(int curLevel, int toX, int toY, int toZ){
	const int toLevel = worldGetFluid(toX,toY,toZ);
	const u8 toType = toLevel & 0xF;
	const u8 toAmount = toLevel & 0xF0;
	if(toType && (toType != (curLevel & 0xF))){return curLevel;}
	const u8 toBlock = worldTryB(toX, toY, toZ);
	if(toBlock){return fluidFlowInto(toBlock, toLevel, curLevel, toX, toY, toZ);}
	const int flowAmount = MIN(curLevel & 0xF0, 0xF0 - toAmount);
	const u8 toNew = toType | (0xF0 & (toAmount + flowAmount));
	worldSetFluid(toX, toY, toZ, toNew);
	const int actualFlow = toNew - toLevel;
	curLevel -= actualFlow;
	return curLevel < 0x10 ? 0 : curLevel;
}

static int fluidPhysicsA(chunkOverlay *fluid, chunkOverlay *block, int cx, int cy, int cz){
	int blocksUsed = 0;
	for(int x=0;x<CHUNK_SIZE;x++){
	for(int z=0;z<CHUNK_SIZE;z++){
	for(int y=0;y<CHUNK_SIZE;y++){
		int curLevel = fluid->data[x][y][z];
		if(curLevel < 0x10){continue;}
		blocksUsed++;
		const u8 b = block ? block->data[x][y][z] : 0;
		if(b){
			curLevel = fluidFlowOut(curLevel, b, cx+x, cy+y-1, cz+z);
		}else{
			curLevel = fluidFlowDown(curLevel, cx+x, cy+y-1, cz+z);
			if(((curLevel & 0xF0) == 0x10) && (rngValA(EVAPORATION_CHANCE) == 0)){curLevel = 0;} // Evaporation
			if(curLevel >= 0x20){
				const int sideFlow = MAX(((curLevel & 0xF0) / 4), 0x10);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x-1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z-1, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z+1, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x+1, cy+y, cz+z, sideFlow);
			}
		}
		fluid->data[x][y][z] = curLevel;
	}
	}
	}
	return blocksUsed;
}

static int fluidPhysicsB(chunkOverlay *fluid, chunkOverlay *block, int cx, int cy, int cz){
	int blocksUsed = 0;
	for(int x=CHUNK_SIZE-1;x>=0;x--){
	for(int z=0;z<CHUNK_SIZE;z++){
	for(int y=0;y<CHUNK_SIZE;y++){
		int curLevel = fluid->data[x][y][z];
		if(curLevel < 0x10){continue;}
		blocksUsed++;
		const u8 b = block ? block->data[x][y][z] : 0;
		if(b){
			curLevel = fluidFlowOut(curLevel, b, cx+x, cy+y-1, cz+z);
		}else{
			curLevel = fluidFlowDown(curLevel, cx+x, cy+y-1, cz+z);
			if(((curLevel & 0xF0) == 0x10) && (rngValA(EVAPORATION_CHANCE) == 0)){curLevel = 0;} // Evaporation
			if(curLevel >= 0x20){
				const int sideFlow = MAX(((curLevel & 0xF0) / 4), 0x10);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z-1, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z+1, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x+1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x-1, cy+y, cz+z, sideFlow);
			}
		}
		fluid->data[x][y][z] = curLevel;
	}
	}
	}
	return blocksUsed;
}

static int fluidPhysicsC(chunkOverlay *fluid, chunkOverlay *block, int cx, int cy, int cz){
	int blocksUsed = 0;
	for(int x=CHUNK_SIZE-1;x>=0;x--){
	for(int z=CHUNK_SIZE-1;z>=0;z--){
	for(int y=0;y<CHUNK_SIZE;y++){
		int curLevel = fluid->data[x][y][z];
		if(curLevel < 0x10){continue;}
		blocksUsed++;
		const u8 b = block ? block->data[x][y][z] : 0;
		if(b){
			curLevel = fluidFlowOut(curLevel, b, cx+x, cy+y-1, cz+z);
		}else{
			curLevel = fluidFlowDown(curLevel, cx+x, cy+y-1, cz+z);
			if(((curLevel & 0xF0) == 0x10) && (rngValA(EVAPORATION_CHANCE) == 0)){curLevel = 0;} // Evaporation
			if(curLevel >= 0x20){
				const int sideFlow = MAX(((curLevel & 0xF0) / 4), 0x10);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z+1, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x+1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x-1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z-1, sideFlow);
			}
		}
		fluid->data[x][y][z] = curLevel;
	}
	}
	}
	return blocksUsed;
}

static int fluidPhysicsD(chunkOverlay *fluid, chunkOverlay *block, int cx, int cy, int cz){
	int blocksUsed = 0;
	for(int x=CHUNK_SIZE-1;x>=0;x--){
	for(int z=0;z<CHUNK_SIZE;z++){
	for(int y=0;y<CHUNK_SIZE;y++){
		int curLevel = fluid->data[x][y][z];
		if(curLevel < 0x10){continue;}
		blocksUsed++;
		const u8 b = block ? block->data[x][y][z] : 0;
		if(b){
			curLevel = fluidFlowOut(curLevel, b, cx+x, cy+y-1, cz+z);
		}else{
			curLevel = fluidFlowDown(curLevel, cx+x, cy+y-1, cz+z);
			if(((curLevel & 0xF0) == 0x10) && (rngValA(EVAPORATION_CHANCE) == 0)){curLevel = 0;} // Evaporation
			if(curLevel >= 0x20){
				const int sideFlow = MAX(((curLevel & 0xF0) / 4), 0x10);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x+1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x-1, cy+y, cz+z, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z-1, sideFlow);
				curLevel = fluidFlowHorizontaly(curLevel, cx+x, cy+y, cz+z+1, sideFlow);
			}
		}
		fluid->data[x][y][z] = curLevel;
	}
	}
	}
	return blocksUsed;
}

int fluidPhysics(chunkOverlay *fluid, chunkOverlay *block, int cx, int cy, int cz){
	static int calls = 0;
	switch(++calls & 3){
	default:
	case 0:
		return fluidPhysicsA(fluid,block,cx,cy,cz);
	case 1:
		return fluidPhysicsB(fluid,block,cx,cy,cz);
	case 2:
		return fluidPhysicsC(fluid,block,cx,cy,cz);
	case 3:
		return fluidPhysicsD(fluid,block,cx,cy,cz);
	}
}
