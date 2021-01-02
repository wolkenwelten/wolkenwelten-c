#include "weather.h"

#include "../misc/noise.h"
#include "../network/packet.h"
#include "../network/messages.h"

u8  cloudTex[256][256];
vec cloudOff;
vec windVel,windGVel;
u8  cloudGDensityMin;
u8  cloudDensityMin;
u8  cloudRainDuration;

void weatherInit(){
	generateNoise(0x84407db3, cloudTex);
	windGVel   = vecMulS(vecRng(),1.f/256.f);
	windGVel.y = 0.f;
	windVel    = windGVel;
	cloudGDensityMin  = 154;//rngValA(31)+154;
	cloudDensityMin   = cloudGDensityMin;
	cloudRainDuration = 0;
}

void weatherUpdateAll(){
	static uint calls = 0;
	if(!isClient && (rngValA((1<<18)-1) == 0)){
		windGVel   = vecMulS(vecRng(),1.f/256.f);
		windGVel.y = 0.f;
		weatherSendUpdate(-1);
	}
	if(!isClient && (rngValA((1<<16)-1) == 0)){
		--cloudGDensityMin;
		weatherSendUpdate(-1);
	}
	if(cloudRainDuration){
		if(!isClient){weatherDoRain();}
		if((calls & 0xFF) == 0){
			if(!isClient && (--cloudRainDuration == 0)){weatherSendUpdate(-1);}
		}
	}
	if((!isClient) && (cloudRainDuration == 0) && (cloudDensityMin < 170) && (calls & 0xFF) == 0){
		const uint chance = MAX(2,16 - (170 - cloudDensityMin));
		if(rngValA((1<<chance)-1) == 0){
			cloudRainDuration = rngValA(15)+16;
			weatherSendUpdate(-1);
		}
	}
	if((cloudGDensityMin != cloudDensityMin) && ((calls & 0xFF) == 0)){
		if(cloudGDensityMin > cloudDensityMin){
			cloudDensityMin++;
		}else{
			cloudDensityMin--;
		}
	}
	windVel = vecMulS(vecAdd(vecMulS(windVel,255.f),windGVel),1.f/256.f);
	cloudOff = vecAdd(cloudOff,windVel);
	if(cloudOff.x > 256.f){cloudOff.x -= 256.f;}
	if(cloudOff.y > 256.f){cloudOff.y -= 256.f;}
	if(cloudOff.z > 256.f){cloudOff.z -= 256.f;}
	if(cloudOff.x <   0.f){cloudOff.x += 256.f;}
	if(cloudOff.y <   0.f){cloudOff.y += 256.f;}
	if(cloudOff.z <   0.f){cloudOff.z += 256.f;}

	calls++;
}

void weatherSendUpdate(uint c){
	packet *p = &packetBuffer;

	p->v.f  [0] = cloudOff.x;
	p->v.f  [1] = cloudOff.y;
	p->v.f  [2] = cloudOff.z;

	p->v.f  [3] = windVel.x;
	p->v.f  [4] = windVel.y;
	p->v.f  [5] = windVel.z;

	p->v.f  [6] = windGVel.x;
	p->v.f  [7] = windGVel.y;
	p->v.f  [8] = windGVel.z;

	p->v.u8 [36] = cloudDensityMin;
	p->v.u8 [37] = cloudGDensityMin;
	p->v.u8 [38] = cloudRainDuration;
	p->v.u8 [39] = 0;

	packetQueue(p,43,10*4,c);
}

void weatherRecvUpdate(const packet *p){
	cloudOff = vecNewP(&p->v.f[0]);
	windVel  = vecNewP(&p->v.f[3]);
	windGVel = vecNewP(&p->v.f[6]);

	cloudDensityMin   = p->v.u8[36];
	cloudGDensityMin  = p->v.u8[37];
	cloudRainDuration = p->v.u8[38];
}

void cloudsSetWind(const vec ngv){
	windGVel = ngv;
	if(!isClient){weatherSendUpdate(-1);}
}

void cloudsSetDensity(u8 gd){
	cloudGDensityMin = gd;
	if(!isClient){weatherSendUpdate(-1);}
}
