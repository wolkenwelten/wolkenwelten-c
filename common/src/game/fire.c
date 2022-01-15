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
#include "fire.h"
#include "./blockType.h"
#include "./blockMining.h"
#include "./item.h"
#include "../misc/effects.h"
#include "../world/world.h"


int fireSpreadTo(int f, int x, int y, int z){
	if(f <= 0){return f;}
	const u8 db = worldGetB(x,y,z);
	if(!db){return f;}
	const int df = worldGetFire(x,y,z);
	if(f <= df){return f;}
	if(rngValA(0xFF) > (uint)f){return f;}
	const int ndf = MIN(255,df + f / 16);
	const int diff = ndf - df;
	worldSetFire(x,y,z,ndf);
	return f - diff;
}

int fireTick(chunkOverlay *f, chunkOverlay *fluid, chunkOverlay *block, int cx, int cy, int cz){
	int blocksLit = 0;
	for(int x=0;x<CHUNK_SIZE;x++){
	for(int y=0;y<CHUNK_SIZE;y++){
	for(int z=0;z<CHUNK_SIZE;z++){
		int curLevel = f->data[x][y][z];
		if(!curLevel){continue;}
		blocksLit++;
		curLevel--;

		if(block){
			const u8 b = block->data[x][y][z];
			if(b){
				curLevel += blockTypeGetFireDamage(b);
				if(curLevel >= blockTypeGetFireHealth(b)){
					blockMiningBurnBlock(cx+x,cy+y,cz+z,b);
				}
			}
		}
		if(fluid){
			const u8 fluidLevel = fluid->data[x][y][z] & 0xF0;
			if(fluidLevel){
				const int newLevel = MAX(0,curLevel - fluidLevel);
				const int diff     = curLevel - newLevel;
				const int newFluid = (fluidLevel - diff) | fluid->data[x][y][z];
				fluid->data[x][y][z] = newFluid > 0xF ? newFluid : 0;
				fxFluidVapor(cx+x,cy+y,cz+z,fluid->data[x][y][z] & 0xF, diff);
				curLevel = newLevel;
			}
		}
		if(curLevel > 16){
			curLevel = fireSpreadTo(curLevel,cx+x,cy+y+1,cz+z);
		}
		if(curLevel > 32){
			switch(rngValA(3)){
			case 0:
				curLevel = fireSpreadTo(curLevel,cx+x-1,cy+y,cz+z);
				break;
			case 1:
				curLevel = fireSpreadTo(curLevel,cx+x+1,cy+y,cz+z);
				break;
			case 2:
				curLevel = fireSpreadTo(curLevel,cx+x,cy+y,cz+z-1);
				break;
			case 3:
				curLevel = fireSpreadTo(curLevel,cx+x,cy+y,cz+z+1);
				break;
			}
		}
		if(curLevel > 48){
			curLevel = fireSpreadTo(curLevel,cx+x,cy+y-1,cz+z);
		}

		f->data[x][y][z] = curLevel;
	}
	}
	}
	return blocksLit;
}

void fireBox(u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, u8 strength){
	for(int cx = x;cx < x+w;cx++){
	for(int cy = y;cy < y+h;cy++){
	for(int cz = z;cz < z+d;cz++){
		worldSetFire(cx,cy,cz,strength);
	}
	}
	}
}

void fireBoxExtinguish(u16 x, u16 y, u16 z, u16 w, u16 h, u16 d, u8 strength){
	(void)strength;
	for(int cx = x;cx < x+w;cx++){
	for(int cy = y;cy < y+h;cy++){
	for(int cz = z;cz < z+d;cz++){
		if(!isClient){
			const blockId b = worldTryB(cx,cy,cz);
			if((b == I_Dry_Grass) && (rngValA(1) == 0)){
				worldSetB(cx,cy,cz,I_Grass);
			}
			if(!b){
				worldSetFluid(cx,cy,cz,0xF0);
			}
		}
	}
	}
	}
}
