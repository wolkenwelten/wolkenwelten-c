#include "projectile.h"

#include "../game/character.h"
#include "../sdl/sfx.h"
#include "../gfx/particle.h"

static inline void projectileDrawAssaultBullet(const projectile *p){
	for(float o = 0.f;o<4.f;o+=0.2f){
		newParticleV(vecAdd(p->pos,vecMulS(p->vel,o)), vecZero(), vecZero(),64, -.05f,0xFF2060B0,128);
	}
	for(int ii=0;ii<4;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), vecZero(),256, -6.f,0xFF50A0F0,96);
	}
	for(int ii=0;ii<2;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), vecZero(),384, -8.f,0xFF70B8FF,64);
	}
}

static inline float projectileDrawGuardianProjectile(const projectile *p){
	newParticleV(p->pos, vecZero(), vecMulS(vecRng(),0.000002f),192, -.1f,0xFF3070E0,2048);
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), vecZero(),256, -4.f,0xFF50A0F0,256);
	//newParticleV(p->pos, vecMulS(vecRng(),0.02f), vecZero(),256, -6.f,0xFF70B8FF,128);
	return vecMag(vecSub(p->pos,player->pos));
}

static inline float projectileDrawGuardianBigProjectile(const projectile *p){
	newParticleV(p->pos, vecZero(), vecMulS(vecRng(),0.000002f),192, -.1f,0xFF3070E0,2048);
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), vecZero(),256, -4.f,0xFF50A0F0,256);
	//newParticleV(p->pos, vecMulS(vecRng(),0.02f), vecZero(),256, -6.f,0xFF70B8FF,128);
	return vecMag(vecSub(p->pos,player->pos));
}

void projectileDrawAll(){
	float maxD = 4096.f;
	for(uint i=0;i<(sizeof(projectileList) / sizeof(projectile));i++){
		projectile *p = &projectileList[i];
		switch(p->style){
		default:
		case 0:
			break;
		case 1:
			projectileDrawAssaultBullet(p);
			break;
		case 2: {
			const float d = projectileDrawGuardianProjectile(p);
			maxD = MIN(d,maxD);
			break; }
		case 3: {
			const float d = projectileDrawGuardianBigProjectile(p);
			maxD = MIN(d,maxD);
			break; }
		}
	}
	if(maxD < 128.f){
		sfxLoop(sfxProjectile,((128.f - maxD)/128.F)*0.5f);
	}else{
		sfxLoop(sfxProjectile,0.f);
	}
}
