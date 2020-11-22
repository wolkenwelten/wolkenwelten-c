#include "fire.h"

#include "../gfx/particle.h"
#include "../../../common/src/network/messages.h"

#include <string.h>

void fireNew(u16 x, u16 y, u16 z, i16 strength){
	msgFireUpdate(-1,0,0,x,y,z,strength);
}

static void fireDraw(const fire *f){
	const vec spos = vecNew(f->x,f->y,f->z);
	const float size = (float)(f->strength * 0.01f);
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.00008f,0.f),size, size*0.5f,0xFF60C8FF, 96);
	if(f->strength <  64){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.0001f,0.f),size*0.7f, size*0.65f,0xFF5098FF, 128);
	if(f->strength < 128){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.0001f,0.f),size*0.5f, size*0.75f,0xFF1F38EF, 156);
	if(f->strength < 256){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.0001f,0.f),size*0.5f, size*0.75f,0xFF1F38EF, 178);
	if(f->strength < 512){return;}
	if((rngValR()&0xF) != 0){return;}
	u32 c = 0xFF101820 | (rngValR()&0x0003070F);
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.00005f,0.f),size*0.01f, size*0.2f,c,2048);
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
