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

#include "noise.h"
#include "../../../common/src/misc/misc.h"
#include <stdio.h>
#include <string.h>

u8 tmp[256][256];

static inline void addh(uint x, uint y,u8 h, u8 heightmap[256][256]){
	heightmap[x&0xFF][y&0xFF] += h;
}

static inline u8 gett(uint x,uint y){
	return tmp[x&0xFF][y&0xFF];
}

static inline u8 interpolate(float v1,float v2,float d){
	return (u8)((v1*(1.0-d))+(v2*d));
}

void dumpHeightmap(u8 heightmap[256][256]){
	printf("------------------------------------------------------------\n");
	for(int y=0;y<256;y++){
		for(int x=0;x<256;x++){
			const u8 h = heightmap[x][y];
			if(h > 192){
				putchar('M');
			}else if(h > 128){
				putchar('O');
			}else if(h > 64){
				putchar('o');
			}else if(h > 32){
				putchar('.');
			}else{
				putchar(' ');
			}
		}
		putchar('|');
		putchar('\n');
	}
	putchar('\n');
}

void perlin_step(uint size,uint amp,float steep, u8 heightmap[256][256]){
	if(amp==0){return;}

	for(uint x=0;x<256/size;x++){
		for(uint y=0;y<256/size;y++){
			if((x==0)||(y==0)){
				tmp[x][y] = 0;
			}else{
				tmp[x][y] = rngValM(amp);
			}
		}
	}
	for(uint xs=0;xs<256/size;xs++){
		for(uint ys=0;ys<256/size;ys++){
			for(uint y=0;y<size;y++){
				for(uint x=0;x<size;x++){
					const float d = ((float)x)/((float)size);
					const u8 h1 = interpolate(gett(xs,ys  ),gett(xs+1,ys  ),d);
					const u8 h2 = interpolate(gett(xs,ys+1),gett(xs+1,ys+1),d);
					const u8  h = interpolate(h1,h2,((float)y)/((float)size));
					addh(x+(xs*size),y+(ys*size),h,heightmap);
				}
			}
		}
	}
	if(size>1){
		perlin_step(size/2,((float)amp)*steep,steep,heightmap);
	}
}

void generateNoise(u64 seed, u8 heightmap[256][256]){
	u64 oldSeed = getRNGSeed();
	memset(heightmap,0,256*256);
	memset(tmp,0,sizeof(tmp));

	seedRNG(seed);
	perlin_step(16,142,0.5,heightmap);
	seedRNG(oldSeed);
	//dumpHeightmap(heightmap);
}

void prefill_noise(u8 heightmap[256][256], uint x, uint y, u8 parent[256][256]){
	u8 h1 = parent[ x   &0xFF][ y   &0xFF];
	u8 h2 = parent[(x+1)&0xFF][ y   &0xFF];
	u8 h3 = parent[ x   &0xFF][(y+1)&0xFF];
	u8 h4 = parent[(x+1)&0xFF][(y+1)&0xFF];

	for(int cx=0;cx<256;cx++){
		for(int cy=0;cy<256;cy++){
			const float d = cx / 256.f;
			const unsigned char ha = interpolate(h1,h2,d);
			const unsigned char hb = interpolate(h3,h4,d);
			const unsigned char  h = interpolate(ha,hb,((float)cy)/256.f);
			heightmap[cx][cy] = h;
		}
	}
}

void generateNoiseZoomed(u64 seed, u8 heightmap[256][256], uint x, uint y, u8 parent[256][256]){
	u64 oldSeed = getRNGSeed();
	//fprintf(stderr,"parent[%i][%i] = %i\n",x,y,parent[x][y]);
	prefill_noise(heightmap,x,y,parent);
	memset(heightmap,parent[x][y],256*256);
	memset(tmp,0,sizeof(tmp));
	//dumpHeightmap(parent);

	seedRNG(seed);
	perlin_step(16,26,0.25,heightmap);
	seedRNG(oldSeed);
	//dumpHeightmap(heightmap);
}
