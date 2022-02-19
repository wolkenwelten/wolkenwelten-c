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
#include "lightFast.h"
#include "../game/blockType.h"
#include "../game/item.h"
#include "../game/time.h"
#include "../misc/profiling.h"

#include <string.h>


void lightSunlightChunk(u8 out[48][48][48], const u8 blockData[16][16][16], u8 curLight[16][16], const u8 blockLight[256], const int x, const int y, const int z, const u8 sunlight){
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

void lightSunlightAir(u8 out[48][48][48], u8 curLight[16][16], const int x, const int y, const int z, const u8 sunlight){
	for(int cy=15;cy>=0;cy--){
	for(int cx=0;cx<16;cx++){
	for(int cz=0;cz<16;cz++){
		curLight[cx][cz] = MIN(sunlight, curLight[cx][cz]+2);
		out[cx+x][cy+y][cz+z] = curLight[cx][cz];
	}
	}
	}
}

static void lightSunlight(u8 out[48][48][48],const chunkOverlay *block[3][3][3], const u8 blockLight[256], const u8 sunlight){
	PROFILE_START();
	for(int x = 0;x < 3; x++){
	for(int z = 0;z < 3; z++){
	u8 curLight[16][16];
	memset(curLight, 0, sizeof(curLight));
	for(int y = 2;y >= 0; y--){
		const chunkOverlay *cur = block[x][y][z];
		if(cur){
			lightSunlightChunkISPC(out, cur->data, curLight, blockLight, x*16, y*16, z*16, sunlight);
		}else{
			lightSunlightAirISPC(out, curLight, x*16, y*16, z*16, sunlight);
		}
	}
	}
	}
	PROFILE_STOP();
}

static void lightBlurFast(u8 buf[48][48][48]){
	PROFILE_START();
	lightBlurISPC(buf);
	PROFILE_STOP();
}

static void lightOut(u8 in[48][48][48], chunkOverlay *out){
	for(int x=0;x<16;x++){
	for(int y=0;y<16;y++){
		memcpy(&out->data[x][y][0],&in[16+x][16+y][16],16);
	}
	}
}

void lightTick(chunkOverlay *light, const chunkOverlay *block[3][3][3]){
	static u8 lightBuffer[48][48][48];
	u8 blockLight[256];
	for(int b=0;b<256;b++){
		blockLight[b] = blockTypeGetLightEmission(b);
	}
	const u8 sunlight = gtimeGetBlockBrightness(gtimeGetTimeOfDay());
	(void)lightSunlight;
	PROFILE_START();

	lightSunlight(lightBuffer, block, blockLight, sunlight);
	lightBlurFast(lightBuffer);
	lightOut(lightBuffer,light);
	PROFILE_STOP();
}
