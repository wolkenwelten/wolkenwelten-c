#include "rain.h"

#include "../asm/asm.h"
#include "../game/weather.h"
#include "../network/messages.h"

__attribute__((aligned(32))) glRainDrop glRainDrops[RAIN_MAX];
__attribute__((aligned(32)))   rainDrop   rainDrops[RAIN_MAX];
__attribute__((aligned(32)))      float   rainVel[4];
uint rainCount = 0;

void rainPosUpdate();

void rainNew(vec pos){
	uint i = ++rainCount;
	if(i >= RAIN_MAX){i = rngValA(RAIN_MAX-1); rainCount--;}

	glRainDrops[i] = (glRainDrop){ pos.x, pos.y, pos.z, 256.f };
	  rainDrops[i] = (  rainDrop){ windVel.x, -0.1f, windVel.z, -0.1f };

	if(!isClient){rainSendUpdate(-1,i);}
}

static void rainDel(uint i){
	glRainDrops[i] = glRainDrops[--rainCount];
	  rainDrops[i] =   rainDrops[  rainCount];
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
	rainVel[0] = windVel.x / 48.f;
	rainVel[1] = -0.0005;
	rainVel[2] = windVel.z / 48.f;
	rainVel[3] = 0.f;

	rainPosUpdate();
	for(uint i=0;i<rainCount;i++){
		if((glRainDrops[i].y < 0.f) || (glRainDrops[i].size < 0.f)){
			rainDel(i);
		}
	}
}

void rainSendUpdate(uint c, uint i){
	packet *p = &packetBuffer;

	p->v.f[0] = glRainDrops[i].x;
	p->v.f[1] = glRainDrops[i].y;
	p->v.f[2] = glRainDrops[i].z;

	packetQueue(p,44,3*4,c);
}
