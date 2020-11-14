#include "projectile.h"

#include "../gfx/particle.h"

void projectileDrawAll(){
	for(uint i=0;i<(sizeof(projectileList) / sizeof(projectile));i++){
		projectile *p = &projectileList[i];
		if(p->style == 0){continue;}
		newParticleV(p->pos, vecZero(), vecZero(),128, -1.f,0xFF2060B0,1024);
		newParticleV(vecAdd(p->pos,vecMulS(p->vel,0.5f)), vecZero(), vecZero(),128, -1.f,0xFF2060B0,1024);
		for(int ii=0;ii<4;ii++){
			newParticleV(p->pos, vecMulS(vecRng(),0.01f), vecZero(),256, -6.f,0xFF50A0F0,96);
		}
		for(int ii=0;ii<2;ii++){
			newParticleV(p->pos, vecMulS(vecRng(),0.01f), vecZero(),384, -8.f,0xFF70B8FF,64);
		}
	}
}
