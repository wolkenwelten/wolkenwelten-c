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
	const u8 amount = level & 0xF1;
	for(int i=0;i<4;i++){
		const int by = rngValA(0xFF);
		if(by > amount){continue;}
		const int bx = rngValA(0xFF);
		const int bz = rngValA(0xFF);
		const float px = x + (bx / 256.0);
		const float py = y + (by / 256.0);
		const float pz = z + (bz / 256.0);
		const u32 c = COLOR(0x10, 0x20, 0x80, 0xF0) | rngValA(COLOR(0x0F, 0x1F, 0x7F, 0x0F));
		newParticle(px, py, pz, 0.f, 0.f, 0.f, 256.f, -0.01f, c, 256);
	}
}

static void fluidGenerateChunkParticles(const chunkOverlay *fluid, int cx, int cy, int cz){
	const u8 *d = &fluid->data[0][0][0];
	for(int x = 0; x < CHUNK_SIZE; x++){
	for(int y = 0; y < CHUNK_SIZE; y++){
	for(int z = 0; z < CHUNK_SIZE; z++){
		const u8 level = *d++;
		if((level & 0x1F) == 0){continue;}
		const u8 b = worldGetB(x,y,z);
		if(b){continue;}
		fluidGenerateBlockParticles(cx+x, cy+y, cz+z, level);
	}
	}
	}
}

void fluidGenerateParticles(){
	static int calls = 0;
	PROFILE_START();

	for(int i = calls & 0xF; i < chunkCount; i+= 0x10){
		const chunk *c = &chunkList[i];
		if((c->nextFree) || (c->fluid == NULL)){continue;}
		fluidGenerateChunkParticles(c->fluid, c->x, c->y, c->z);
	}
	++calls;
	PROFILE_STOP();
}
