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

#include "../main.h"
#include "../game/character/character.h"
#include "../gfx/particle.h"
#include "../gfx/gfx.h"
#include "../gui/overlay.h"
#include "../sdl/sdl.h"
#include "../sfx/sfx.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/chunkOverlay.h"
#include "../../../common/src/game/fire.h"
#include "../../../common/src/misc/colors.h"
#include "../../../common/src/misc/profiling.h"

#include <string.h>


static void fireGenerateParticles(int x, int y, int z, u8 strength){
	const int bx = rngValA(0xFF);
	const int by = rngValA(0xFF);
	const int bz = rngValA(0xFF);
	const float px = x + (bx / 256.0);
	const float py = y + (by / 256.0);
	const float pz = z + (bz / 256.0);

	const vec spos = vecNew(px,py,pz);
	const float size = MAX(3.f,(float)(strength * 0.03f));

	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size, size*0.5f,0xFF60C8FF, 96);
	if(strength <  16){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size*0.7f, size*0.65f,0xFF5098FF, 128);
	if(strength < 48){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size*0.6f, size*0.75f,0xFF1F38EF, 156);
	if(strength < 128){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size*0.5f, size*0.75f,0xFF1F38EF, 178);
	if(strength < 160){return;}
	u32 c = 0x00101820 | (rngValR()&0x0003070F);
	newSparticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.001f,0),vecMulS(vecRng(),0.0001f)), size*0.01f, size*0.2f,c,2048);
}

static void fireGenerateChunkParticles(const chunkOverlay *f, int cx, int cy, int cz){
	if(player){
		if(abs(cx-(int)player->pos.x) > renderDistance){return;}
		if(abs(cy-(int)player->pos.y) > renderDistance){return;}
		if(abs(cz-(int)player->pos.z) > renderDistance){return;}
	}

	const u8 *d = &f->data[0][0][0];
	for(int x = 0; x < CHUNK_SIZE; x++){
	for(int y = 0; y < CHUNK_SIZE; y++){
	for(int z = 0; z < CHUNK_SIZE; z++){
		const u8 strength = *d++;
		if(!strength){continue;}
		fireGenerateParticles(cx+x, cy+y, cz+z, strength);
	}
	}
	}
}

void fireDrawAll(){
	static int calls = 0;
	PROFILE_START();

	for(uint i = calls&0x7; i < chungusCount; i+=8){
		chungus *cng = &chungusList[i];
		if(!chungusInFrustum(cng)){continue;}
		for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
		for(int z=0;z<16;z++){
			const chunk *c = &cng->chunks[x][y][z];
			if(c->flame == NULL){continue;}
			if(!chunkInFrustum(c)){continue;}
			fireGenerateChunkParticles(c->flame, c->x, c->y, c->z);
		}
		}
		}
	}
	++calls;
	PROFILE_STOP();
}

void fireUpdateAll(){
	static int calls = 0;
	PROFILE_START();

	for(uint i = calls&0xF; i < chungusCount; i+=0x10){
		chungus *cng = &chungusList[i];
		for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
		for(int z=0;z<16;z++){
			chunk *c = &cng->chunks[x][y][z];
			if(c->flame== NULL){continue;}
			if(!fireTick(c->flame, c->fluid, c->block, c->x, c->y, c->z)){
				chunkOverlayFree(c->flame);
				c->flame = NULL;
			}
		}
		}
		}
	}
	++calls;

	PROFILE_STOP();
}
