#include "weather.h"

#include "../misc/noise.h"
#include "../misc/profiling.h"
#include "../network/packet.h"
#include "../network/messages.h"

u8  cloudTex[256][256];
vec cloudOff;
vec windVel,windGVel;
u8  cloudGDensityMin;
u8  cloudDensityMin;
u8  rainDuration;

void weatherInit(){
	generateNoise(0x84407db3, cloudTex);
	windGVel   = vecMulS(vecRng(),1.f/256.f);
	windGVel.y = 0.f;
	windVel    = windGVel;
	cloudGDensityMin  = 154;
	cloudDensityMin   = cloudGDensityMin;
	rainDuration = 0;
}

void weatherUpdateAll(){
	static uint calls = 0;
	PROFILE_START();

	if(!isClient && (rngValA((1<<18)-1) == 0)){
		windGVel   = vecMulS(vecRng(),1.f/256.f);
		windGVel.y = 0.f;
		weatherSendUpdate(-1);
	}
	if(!isClient && (rngValA((1<<16)-1) == 0)){
		--cloudGDensityMin;
		weatherSendUpdate(-1);
	}
	if(rainDuration){
		if(!isClient){weatherDoRain();}
		if((calls & 0xFFF) == 0){
			if((--rainDuration == 0) && !isClient){weatherSendUpdate(-1);}
		}else if(((calls & 0xFFF) == 1) && !isClient){
			cloudGDensityMin++;
			weatherSendUpdate(-1);
		}
	}
	if((!isClient) && (rainDuration == 0) && (cloudDensityMin < 170) && (calls & 0xFFF) == 0){
		const uint chance = MAX(2,16 - (170 - cloudDensityMin));
		if(rngValA((1<<chance)-1) == 0){
			rainDuration = rngValA(15)+16;
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
	PROFILE_STOP();
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
	p->v.u8 [38] = rainDuration;
	p->v.u8 [39] = 0;

	packetQueue(p,43,10*4,c);
}

void weatherRecvUpdate(const packet *p){
	cloudOff = vecNewP(&p->v.f[0]);
	windVel  = vecNewP(&p->v.f[3]);
	windGVel = vecNewP(&p->v.f[6]);

	cloudDensityMin   = p->v.u8[36];
	cloudGDensityMin  = p->v.u8[37];
	rainDuration = p->v.u8[38];
}

void cloudsSetWind(const vec ngv){
	windGVel = ngv;
	if(!isClient){weatherSendUpdate(-1);}
}

void cloudsSetDensity(u8 gd){
	cloudGDensityMin = gd;
	if(!isClient){weatherSendUpdate(-1);}
}

void weatherSetRainDuration(u8 dur){
	rainDuration = dur;
	if(!isClient){weatherSendUpdate(-1);}
}
