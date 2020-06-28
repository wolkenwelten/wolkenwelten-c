#include "noise.h"
#include <string.h>
#include "../misc/misc.h"

unsigned char heightmap[256][256];
unsigned char tmp[256][256];

inline void addh(int x,int y,unsigned char h){
	heightmap[x&0xFF][y&0xFF] += h;
}

inline unsigned char gett(int x,int y){
	return tmp[x&0xFF][y&0xFF];
}

inline unsigned char geth(int x,int y){
	return heightmap[x&0xFF][y&0xFF];
}

inline unsigned char interpolate(float v1,float v2,float d){
	return (unsigned char)((v1*(1.0-d))+(v2*d));
}

void perlin_step(int size,int amp,float steep){
	if(amp==0){return;}

	for(int x=0;x<256/size;x++){
		for(int y=0;y<256/size;y++){
			const unsigned char h = rngValM(amp);
			if((x==0)||(y==0)){
				tmp[x][y] = 0;
			}else{
				tmp[x][y] = h;
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
					addh(x+(xs*size),y+(ys*size),h);
				}
			}
		}
	}
	if(size>1){
		perlin_step(size/2,(int)(((float)amp)*steep),steep);
	}
}

void generateNoise(unsigned int seed){
	unsigned int oldSeed = getRNGSeed();
	memset(heightmap,0,sizeof(heightmap));
	memset(tmp,0,sizeof(tmp));

	seedRNG(seed);
	perlin_step(16,142,0.5);
	seedRNG(oldSeed);
}
