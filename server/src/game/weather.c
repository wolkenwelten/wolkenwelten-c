#include "weather.h"

#include "../game/rain.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"

void weatherDoRain(){
	static uint calls = 0;
	const ivec toff = ivecNewV(vecFloor(cloudOff));
	if((++calls & 0x7) != 0){return;}
	for(uint i=0;i<chungusCount;i++){
		const chungus *c = &chungusList[i];
		if(c->y & 1){continue;}
		const vec cpos = vecNew(c->x << 8, c->y << 8, c->z << 8);
		u8 x = rngValA(255);
		u8 z = rngValA(255);
		const int tx = (x-toff.x) & 0xFF;
		const int tz = (z-toff.z) & 0xFF;
		int v = cloudTex[tx][tz];
		if(v > (cloudDensityMin+16)){continue;}
		const vec rpos = vecAdd(cpos,vecNew(x,32.f,z));
		rainNew(rpos);
	}
}
