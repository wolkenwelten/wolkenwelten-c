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
#include "light.h"
#include "../asm/asm.h"
#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/item.h"
#include "../game/time.h"
#include "../misc/profiling.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

extern character *player;

static void lightBlurZ(u8 out[48][48][48]){
	PROFILE_START();
	for(int x=0;x < 48;x++){
	for(int y=0;y < 48;y++){
	i8 a = 0;
	i8 b = 0;
	for(int z=0;z < 32;z++){
		a = MAX(out[x][y][z], a);
		out[x][y][z] = a;
		a = MAX(a - 2, 0);

		b = MAX(out[x][y][47-z], b);
		out[x][y][47-z] = b;
		b = MAX(b - 2, 0);
	}
	}
	}
	PROFILE_STOP();
}

void lightBlurYPortable(u8 out[48][48][48]){
	for(int x=0;x < 48;x++){
	for(int z=0;z < 48;z++){
	i8 a = 0;
	i8 b = 0;
	for(int y=0;y < 32;y++){
		a = MAX(out[x][y][z], a);
		out[x][y][z] = a;
		a = MAX(a - 2, 0);

		b = MAX(out[x][47-y][z], b);
		out[x][47-y][z] = b;
		b = MAX(b - 2, 0);
	}
	}
	}
}

void lightBlurXPortable(u8 out[48][48][48]){
	PROFILE_START();
	for(int y=0;y < 48;y++){
	for(int z=0;z < 48;z++){
	i8 a = 0;
	i8 b = 0;
	for(int x=0;x < 32;x++){
		a = MAX(out[x][y][z], a);
		out[x][y][z] = a;
		a = MAX(a - 2, 0);

		b = MAX(out[47-x][y][z], b);
		out[47-x][y][z] = b;
		b = MAX(b - 2, 0);
	}
	}
	}
	PROFILE_STOP();
}

static void lightBlur(u8 buf[48][48][48]){
	PROFILE_START();
	lightBlurZ(buf);
	lightBlurX(buf);
	lightBlurY(buf);
	PROFILE_STOP();
}

static void lightSunlightChunk(u8 out[48][48][48], const u8 blockData[16][16][16], u8 curLight[16][16], const u8 blockLight[256], const int x, const int y, const int z, const u8 sunlight){
	for(int cy=15;cy>=0;cy--){
	for(int cx=0;cx<16;cx++){
	for(int cz=0;cz<16;cz++){
		const u8 b = blockData[cx][cy][cz];
		if(b){
			curLight[cx][cz] = 0;
			out[x+cx][y+cy][z+cz] = blockLight[b];
		}else{
			out[x+cx][y+cy][z+cz] = curLight[cx][cz];
			curLight[cx][cz] = MIN(sunlight, curLight[cx][cz]+2);
		}
	}
	}
	}
}

static void lightSunlightAir(u8 out[48][48][48], u8 curLight[16][16], const int x, const int y, const int z, const u8 sunlight){
	for(int cy=15;cy>=0;cy--){
	for(int cx=0;cx<16;cx++){
	for(int cz=0;cz<16;cz++){
		curLight[cx][cz] = MIN(sunlight, curLight[cx][cz]+2);
		out[cx+x][cy+y][cz+z] = curLight[cx][cz];
	}
	}
	}
}

static void lightSunlight(u8 out[48][48][48],const chunkOverlay *block[3][3][3]){
	PROFILE_START();

	u8 blockLight[256];
	for(int b=0;b<256;b++){
		blockLight[b] = blockTypeGetLightEmission(b);
	}
	const u8 sunlight = gtimeGetBlockBrightness(gtimeGetTimeOfDay());

	for(int x = 0;x < 3; x++){
	for(int z = 0;z < 3; z++){
	u8 curLight[16][16];
	memset(curLight, 0, sizeof(curLight));
	for(int y = 2;y >= 0; y--){
		const chunkOverlay *cur = block[x][y][z];
		if(cur){
			lightSunlightChunk(out, cur->data, curLight, blockLight, x*16, y*16, z*16, sunlight);
		}else{
			lightSunlightAir(out, curLight, x*16, y*16, z*16, sunlight);
		}
	}
	}
	}
	PROFILE_STOP();
}

static void lightOut(u8 in[48][48][48], chunkOverlay *out){
	for(int x=0;x<16;x++){
	for(int y=0;y<16;y++){
		memcpy(&out->data[x][y][0],&in[16+x][16+y][16],16);
	}
	}
}

u8 lightBuffer[48][48][48];
void lightTick(chunkOverlay *light, const chunkOverlay *block[3][3][3]){
	PROFILE_START();
	lightSunlight(lightBuffer, block);
	lightBlur(lightBuffer);
	lightOut(lightBuffer,light);
	PROFILE_STOP();
}
