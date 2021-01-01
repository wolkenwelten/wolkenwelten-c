#include "clouds.h"
#include "../misc/noise.h"
#include "../network/packet.h"
#include "../network/messages.h"

u8  cloudTex[256][256];
vec cloudOff;
vec windVel,windGVel;
u8 cloudGDensityMin;
u8 cloudDensityMin;

void cloudsInit(){
	generateNoise(0x84407db3, cloudTex);
	windGVel   = vecMulS(vecRng(),1.f/256.f);
	windGVel.y = 0.f;
	windVel    = windGVel;
	cloudGDensityMin = 154;//rngValA(31)+154;
	cloudDensityMin  = cloudGDensityMin;
}

void cloudsUpdateAll(){
	static uint calls = 0;
	if(!isClient && (rngValA((1<<14)-1) == 0)){
		windGVel   = vecMulS(vecRng(),1.f/256.f);
		windGVel.y = 0.f;
		cloudsSendUpdate(-1);
	}
	if(!isClient && (rngValA((1<<14)-1) == 0)){
		cloudGDensityMin = rngValA(31)+154;
		cloudsSendUpdate(-1);
	}
	if((cloudGDensityMin != cloudDensityMin) && ((++calls & 0xFF) == 0)){
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
}

void cloudsSendUpdate(uint c){
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
	p->v.u8 [38] = 0;
	p->v.u8 [39] = 0;

	packetQueue(p,43,10*4,c);
}

void cloudsRecvUpdate(const packet *p){
	cloudOff = vecNewP(&p->v.f[0]);
	windVel  = vecNewP(&p->v.f[3]);
	windGVel = vecNewP(&p->v.f[6]);

	cloudDensityMin  = p->v.u8[36];
	cloudGDensityMin = p->v.u8[37];
}

void cloudsSetWind(const vec ngv){
	windGVel = ngv;
	if(!isClient){
		cloudsSendUpdate(-1);
	}
}

void cloudsSetDensity(u8 gd){
	cloudGDensityMin = gd;
	if(!isClient){
		cloudsSendUpdate(-1);
	}
}
