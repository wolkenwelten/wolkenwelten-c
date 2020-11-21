#include "fire.h"

#include "../gfx/particle.h"
#include "../../../common/src/network/messages.h"

#include <string.h>

void fireNew(const vec pos, u16 strength){
	msgFireUpdate(-1,0,0,pos.x,pos.y,pos.z,strength);
}

static void fireDraw(const fire *f){
	u32 c = 0;
	const vec pos = vecAdd(vecNew(f->x,f->y,f->z),vecNew(0.5f,0.5f,0.5f));
	newParticleV(pos, vecMulS(vecRng(),0.01f), vecNew(0.f,0.0001f,0.f),128, 2.f,0xFF60C8FF, 256);
	newParticleV(pos, vecMulS(vecRng(),0.009f), vecNew(0.f,0.0001f,0.f), 96, 1.5f,0xFF5098FF, 512);
	newParticleV(pos, vecMulS(vecRng(),0.007f), vecNew(0.f,0.0001f,0.f), 16, 1.0f,0xFF1F38EF, 768);
	c = 0xFF101820 | (rngValR()&0x0003070F);
	newParticleV(pos, vecMulS(vecRng(),0.005f), vecNew(0.f,0.00005f,0.f),  1, .5f,c,4096);
}

void fireDrawAll(){
	for(uint i=0;i<fireCount;i++){
		fireDraw(&fireList[i]);
	}
}

void fireRecvUpdate(uint c, const packet *p){
	(void)c;
	const uint i     = p->v.u16[0];
	const uint count = p->v.u16[1];
	if(count > fireCount){
		memset(&fireList[fireCount],0,sizeof(fire) * (count-fireCount));
	}
	fireCount = count;
	fire *f = &fireList[i];
	f->x = p->v.u16[2];
	f->y = p->v.u16[3];
	f->z = p->v.u16[4];
	f->strength = p->v.i16[5];
}
