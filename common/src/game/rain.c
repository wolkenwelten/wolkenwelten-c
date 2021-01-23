#include "rain.h"

#include "../asm/asm.h"
#include "../game/water.h"
#include "../game/weather.h"
#include "../misc/profiling.h"
#include "../mods/api_v1.h"
#include "../network/messages.h"

__attribute__((aligned(32))) glRainDrop glRainDrops[RAIN_MAX];
__attribute__((aligned(32)))   rainDrop   rainDrops[RAIN_MAX];
__attribute__((aligned(32)))      float   rainVel[4];
                                    u64   rainCoords[RAIN_MAX];
uint rainCount = 0;


void rainPosUpdate();
void fxRainDrop(const vec pos);

void rainNew(vec pos){
	uint i = ++rainCount;
	if(i >= RAIN_MAX){i = rngValA(RAIN_MAX-1); rainCount--;}

	glRainDrops[i]  = (glRainDrop){ pos.x, pos.y, pos.z, 256.f };
	  rainDrops[i]  = (  rainDrop){ windVel.x, -0.1f, windVel.z, -0.1f };
          rainCoords[i] = 0;

	if(!isClient){rainSendUpdate(-1,i);}
}

static void rainDel(uint i){
	glRainDrops[i]  = glRainDrops[--rainCount];
	  rainDrops[i]  =   rainDrops[  rainCount];
	  rainCoords[i] =   rainCoords[ rainCount];
}

#ifndef WW_ASM_RAIN_POS_UPDATE
void rainPosUpdate(){
	for(uint i=0;i<rainCount;i++){
		glRainDrops[i].x     += rainDrops[i].vx;
		glRainDrops[i].y     += rainDrops[i].vy;
		glRainDrops[i].z     += rainDrops[i].vz;
		glRainDrops[i].size  += rainDrops[i].vsize;

		  rainDrops[i].vx    += rainVel[0];
		  rainDrops[i].vy    += rainVel[1];
		  rainDrops[i].vz    += rainVel[2];
		  rainDrops[i].vsize += rainVel[3];
	}
}
#endif

void rainUpdateAll(){
	PROFILE_START();

	rainVel[0] = windVel.x / 40.f;
	rainVel[1] = -0.0005;
	rainVel[2] = windVel.z / 40.f;
	rainVel[3] = 0.f;

	rainPosUpdate();
	for(uint i=rainCount-1;i<rainCount;i--){
		const glRainDrop *glrd = &glRainDrops[i];
		if((glrd->y < 0.f) || (glrd->size < 0.f)){
			rainDel(i);
			continue;
		}
		const u64 newCoords = ((u64)glrd->x & 0xFFFF) | (((u64)glrd->y & 0xFFFF) << 16) | (((u64)glrd->z & 0xFFFF) << 32);
		if(newCoords != rainCoords[i]){
			if(!worldIsLoaded(glrd->x,glrd->y,glrd->z)){
				rainDel(i);
				continue;
			}
			if(worldGetB(glrd->x,glrd->y,glrd->z) != 0){
				if(isClient){
					fxRainDrop(vecNew(glrd->x,glrd->y,glrd->z));
				}else{
					fireBoxExtinguish (glrd->x-1, glrd->y-1, glrd->z-1, 3, 3, 3, 256);
					/*
					waterNew(glrd->x,glrd->y,glrd->z,512);
					waterNew(glrd->x,glrd->y-1,glrd->z,512);
					*/
				}
				rainDel(i);
				continue;
			}
			rainCoords[i] = newCoords;
		}
	}

	PROFILE_STOP();
}

void rainSendUpdate(uint c, uint i){
	packet *p = &packetBuffer;

	p->v.f[0] = glRainDrops[i].x;
	p->v.f[1] = glRainDrops[i].y;
	p->v.f[2] = glRainDrops[i].z;

	packetQueue(p,44,3*4,c);
}
