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
#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/item.h"
#include "../game/time.h"
#include "../misc/profiling.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

extern character *player;

#define LIGHT_SIZE (CHUNK_SIZE * 3)

static int lightOff(int x, int y, int z){
	return z + (y*LIGHT_SIZE) + (x*LIGHT_SIZE*LIGHT_SIZE);
}

static void lightBlurZ(u8 *out){
        PROFILE_START();
	for(int x=0;x < LIGHT_SIZE;x++){
	for(int y=0;y < LIGHT_SIZE;y++){
	i8 a = 0;
        i8 b = 0;
	for(int z=0;z < LIGHT_SIZE;z++){
                int off = lightOff(x,y,z);
		a = out[off] = MAX(out[off], a);
		a = MAX(a - 2, 0);

                int bff = lightOff(x,y,(LIGHT_SIZE-1)-z);
                b = out[bff] = MAX(out[bff], b);
		b = MAX(b - 2, 0);
	}
	}
	}
        PROFILE_STOP();
}

static void lightBlurY(u8 *out){
        PROFILE_START();
	for(int x=0;x < LIGHT_SIZE;x++){
	for(int z=0;z < LIGHT_SIZE;z++){
	i8 a = 0;
        i8 b = 0;
	for(int y=0;y < LIGHT_SIZE;y++){
		const int off = lightOff(x,y,z);
		a = out[off] = MAX(out[off], a);
		a = MAX(a - 2, 0);

                const int bff = lightOff(x,(LIGHT_SIZE-1) - y,z);
		b = out[bff] = MAX(out[bff], b);
		b = MAX(b - 2, 0);
	}
	}
	}
        PROFILE_STOP();
}

static void lightBlurX(u8 *out){
        PROFILE_START();
	for(int y=0;y < LIGHT_SIZE;y++){
	for(int z=0;z < LIGHT_SIZE;z++){
	i8 a = 0;
        i8 b = 0;
	for(int x=0;x < LIGHT_SIZE;x++){
		const int off = lightOff(x,y,z);
		a = out[off] = MAX(out[off], a);
		a = MAX(a - 2, 0);

                const int bff = lightOff((LIGHT_SIZE-1) - x,y,z);
		b = out[bff] = MAX(out[bff], b);
		b = MAX(b - 2, 0);
	}
	}
	}
        PROFILE_STOP();
}

static void lightBlur(u8 *buf){
        PROFILE_START();
        lightBlurZ(buf);
	lightBlurX(buf);
	lightBlurY(buf);
        PROFILE_STOP();
}

static void lightSunlight(u8 *out,const chunkOverlay *block[3][3][3]){
        PROFILE_START();
	const u8 sunlight = gtimeGetBlockBrightness(gtimeGetTimeOfDay());
	for(int x=0;x<LIGHT_SIZE;x++){
                const u8 cx = x & 0xF;
	for(int z=0;z<LIGHT_SIZE;z++){
                const u8 cz = z & 0xF;
                u8 curLight = sunlight;
	for(int cy=2;cy>=0;cy--){
		const chunkOverlay *cur = block[x/CHUNK_SIZE][cy][z/CHUNK_SIZE];
                const int ccy = cy * CHUNK_SIZE;
                if(cur){
                        for(int y=CHUNK_SIZE-1;y>=0;y--){
                                const int off = lightOff(x,y+ccy,z);
                                const blockId b = cur->data[cx][y][cz];
                                if(b){
                                        out[off] = blockTypeGetLightEmission(b);
                                        curLight = 0;
                                }else{
                                        out[off] = curLight = MIN(sunlight, curLight+2);
                                }
                        }
                }else{
                        for(int y=CHUNK_SIZE-1;y>=0;y--){
                                const int off = lightOff(x,y+ccy,z);
                                out[off] = curLight = MIN(sunlight, curLight+2);
                        }
                }
	}
	}
	}
        PROFILE_STOP();
}

static void lightOut(u8 *in, chunkOverlay *out){
        u64 *o = (u64 *)&out->data[0][0][0];
	for(int x=CHUNK_SIZE;x<CHUNK_SIZE*2;x++){
	for(int y=CHUNK_SIZE;y<CHUNK_SIZE*2;y++){
                const u64 *i = (const u64 *)&in[lightOff(x,y,CHUNK_SIZE)];
		*o++ = *i++;
		*o++ = *i++;
	}
	}
}

u8 lightBuffer[LIGHT_SIZE * LIGHT_SIZE * LIGHT_SIZE];
void lightTick(chunkOverlay *light, const chunkOverlay *block[3][3][3]){
	PROFILE_START();
	lightSunlight(lightBuffer, block);
	lightBlur(lightBuffer);
        lightOut(lightBuffer,light);
	PROFILE_STOP();
}
