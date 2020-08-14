#include "noise.h"
#include "../../../common/src/misc/misc.h"
#include <stdio.h>
#include <string.h>

unsigned char tmp[256][256];

inline void addh(int x, int y,unsigned char h, unsigned char heightmap[256][256]){
	heightmap[x&0xFF][y&0xFF] += h;
}

inline unsigned char gett(int x,int y){
	return tmp[x&0xFF][y&0xFF];
}

inline unsigned char interpolate(float v1,float v2,float d){
	return (unsigned char)((v1*(1.0-d))+(v2*d));
}

void dumpHeightmap(unsigned char heightmap[256][256]){
	printf("------------------------------------------------------------\n");
	for(int y=0;y<256;y++){
		for(int x=0;x<256;x++){
			const unsigned char h = heightmap[x][y];
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

void perlin_step(int size,int amp,float steep, unsigned char heightmap[256][256]){
	if(amp==0){return;}

	for(int x=0;x<256/size;x++){
		for(int y=0;y<256/size;y++){
			if((x==0)||(y==0)){
				tmp[x][y] = 0;
			}else{
				tmp[x][y] = rngValM(amp);
			}
		}
	}
	for(int xs=0;xs<256/size;xs++){
		for(int ys=0;ys<256/size;ys++){
			for(int y=0;y<size;y++){
				for(int x=0;x<size;x++){
					const float d = ((float)x)/((float)size);
					const unsigned char h1 = interpolate(gett(xs,ys  ),gett(xs+1,ys  ),d);
					const unsigned char h2 = interpolate(gett(xs,ys+1),gett(xs+1,ys+1),d);
					const unsigned char  h = interpolate(h1,h2,((float)y)/((float)size));
					addh(x+(xs*size),y+(ys*size),h,heightmap);
				}
			}
		}
	}
	if(size>1){
		perlin_step(size/2,(int)(((float)amp)*steep),steep,heightmap);
	}
}

void generateNoise(unsigned int seed, unsigned char heightmap[256][256]){
	unsigned int oldSeed = getRNGSeed();
	memset(heightmap,0,256*256);
	memset(tmp,0,sizeof(tmp));

	seedRNG(seed);
	perlin_step(16,142,0.5,heightmap);
	seedRNG(oldSeed);
	//dumpHeightmap(heightmap);
}

void prefill_noise(unsigned char heightmap[256][256], int x, int y, unsigned char parent[256][256]){
	unsigned char h1 = parent[ x   &0xFF][ y   &0xFF];
	unsigned char h2 = parent[(x+1)&0xFF][ y   &0xFF];
	unsigned char h3 = parent[ x   &0xFF][(y+1)&0xFF];
	unsigned char h4 = parent[(x+1)&0xFF][(y+1)&0xFF];
	
	for(int cx=0;cx<256;cx++){
		for(int cy=0;cy<256;cy++){
			const float d = (float)cx / 256.f;
			const unsigned char ha = interpolate(h1,h2,d);
			const unsigned char hb = interpolate(h3,h4,d);
			const unsigned char  h = interpolate(ha,hb,((float)cy)/256.f);
			heightmap[cx][cy] = h;
		}
	}
}

void generateNoiseZoomed(unsigned int seed, unsigned char heightmap[256][256], int x, int y, unsigned char parent[256][256]){
	unsigned int oldSeed = getRNGSeed();
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