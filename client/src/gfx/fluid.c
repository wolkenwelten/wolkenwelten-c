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
#include "../gfx/particle.h"
#include "../voxel/chunk.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/colors.h"
#include "../../../common/src/misc/profiling.h"

static void fluidGenerateBlockParticles(int x, int y, int z, u8 level){
	const u8 amount = (level & 0xF0);
	//for(int i=0;i<2;i++){
		const int by = rngValA(0xFF);
		//if(by > amount){continue;}
		const int bx = rngValA(0xFF);
		const int bz = rngValA(0xFF);
		const float px = x + (bx / 256.0);
		const float py = y + (by / 256.0);
		const float pz = z + (bz / 256.0);
		u32 c;
		if(amount == 0x10){
			c = COLOR(0x60, 0xA0, 0xE0, 0xF0) | rngValA(COLOR(0x0F, 0x1F, 0x1F, 0x0F));
		}else if(amount < (0x10*4)){
			c = COLOR(0x20, 0x40, 0xC0, 0xF0) | rngValA(COLOR(0x0F, 0x1F, 0x3F, 0x0F));
		}else if(amount == 0xF0){
			c = COLOR(0x18, 0x18, 0x70, 0xF0) + rngValA(COLOR(0x07, 0x0F, 0x3F, 0x0F));
		}else{
			c = COLOR(0x10, 0x20, 0x80, 0xF0) | rngValA(COLOR(0x0F, 0x1F, 0x7F, 0x0F));
		}
		newParticle(px, py, pz, 0.f, 0.f, 0.f, 256.f, -0.01f, c, 128);
	//}
}

static void fluidGenerateChunkParticles(const chunkOverlay *fluid, const chunkOverlay *block, int cx, int cy, int cz){
	const u8 *d = &fluid->data[0][0][0];
	for(int x = 0; x < CHUNK_SIZE; x++){
	for(int y = 0; y < CHUNK_SIZE; y++){
	for(int z = 0; z < CHUNK_SIZE; z++){
		const u8 level = *d++;
		if((level & 0xF0) == 0){continue;}
		if(block->data){
			const u8 b = block->data[x][y][z];
			if(b){continue;}
		}
		fluidGenerateBlockParticles(cx+x, cy+y, cz+z, level);
	}
	}
	}
}

void fluidGenerateParticles(){
	static int calls = 0;
	PROFILE_START();

	//for(uint i = calls&0xF; i < chungusCount; i+=0x10){
	for(uint i = 0; i < chungusCount; i++){
		chungus *cng = &chungusList[i];
		for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
		for(int z=0;z<16;z++){
			const chunk *c = &cng->chunks[x][y][z];
			if(c->fluid == NULL){continue;}
			fluidGenerateChunkParticles(c->fluid, c->block, c->x, c->y, c->z);
		}
		}
		}
	}
	++calls;
	PROFILE_STOP();
}
