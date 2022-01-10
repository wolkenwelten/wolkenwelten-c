/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "projectile.h"

#include "../game/character.h"
#include "../sfx/sfx.h"
#include "../gfx/particle.h"

static inline void projectileDrawFlameBullet(const projectile *p){
	newParticleV(p->pos, vecMulS(vecRng(),0.012f), 178, -2.f,0xFF60C8FF,78);
	newParticleV(p->pos, vecMulS(vecRng(),0.006f), 128, -1.f,0xFF5098FF,64);
	newParticleV(p->pos, vecMulS(vecRng(),0.002f), 192, -3.f,0xFF1F38EF,96);
	newParticleV(p->pos, vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.01f)), 1.f, 2.5f,0xFF1F38EF, 156);
	const u32 c = 0x00101820 | (rngValR()&0x0003070F);
	newSparticleV(vecAdd(p->pos,vecRngAbs()), vecAdd(vecNew(0,0.001f,0),vecMulS(vecRng(),0.0001f)), 0.1f, 2.f,c,384);
}

static inline void projectileDrawWaterBullet(const projectile *p){
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), 128, -1.f,0xFFB05020,192);
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), 178, -2.f,0xFF9F7830,128);
}

static inline void projectileDrawAssaultBullet(const projectile *p){
	vec cp,cv;
	const u32 colors[] = {
		0x00010713,
		0x00031F39,
		0x000F0B1B,
		0x000C1D38
	};
	u32 cc = colors[(p - projectileList)&3];
	cp = p->pos;
	cv = vecMulS(p->vel,-0.4f);
	for(int ii=0;ii<20;ii++){
		cp = vecAdd(cp,cv);
		newParticleV(cp, vecMulS(vecRng(),0.001f), 156, -6.f,0xFF50A0F0 | cc,48);
	}
	cp = p->pos;
	cv = vecMulS(p->vel,-0.2f);
	for(int i=0;i<100;i++){
		cp = vecAdd(cp,cv);
		newParticleV(cp, vecZero(), 64, -.4f,0xFF2060C0 | cc,64);
	}

	for(int ii=0;ii<4;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 192, -6.f,0xFF50A0F0 | cc,78);
	}
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -8.f,0xFF70B8FF | cc,96);
}

static inline void projectileDrawShotgunBullet(const projectile *p){
	vec cp,cv;
	u32 tc = 0xFF2060C0 | (rngValR() & 0x070F3F);
	cp = p->pos;
	cv = vecMulS(p->vel,-0.5f);
	for(int i=0;i<40;i++){
		cp = vecAdd(cp,cv);
		newParticleV(cp, vecZero(), 64, -.4f,tc,128);
	}
	for(int ii=0;ii<4;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -6.f,0xFF50D0F0,78);
	}
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), 384, -8.f,0xFF90E8FF,48);
}

static inline float projectileDrawGuardianProjectile(const projectile *p){
	newParticleV(p->pos, vecZero(), 192, -.1f,0xFF3070E0,1024);
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -4.f,0xFF50A0F0,256);
	return vecMag(vecSub(p->pos,player->pos));
}

static inline float projectileDrawGuardianBigProjectile(const projectile *p){
	newParticleV(p->pos, vecZero(), 384, -.1f,0xFF205FE9,2048);
	newParticleV(p->pos, vecMulS(vecRng(),0.015f), 512, -4.f,0xFF4670FF,256);
	return vecMag(vecSub(p->pos,player->pos));
}

void projectileDrawAll(){
	float maxD = 4096.f;
	for(uint i=0;i<countof(projectileList);i++){
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
		case 4:
			projectileDrawShotgunBullet(p);
			break;
		case 5:
			projectileDrawFlameBullet(p);
			break;
		case 6:
			projectileDrawWaterBullet(p);
			break;
		}
	}
	if(maxD < 128.f){
		sfxLoop(sfxProjectile,((128.f - maxD)/128.F)*0.5f);
	}else{
		sfxLoop(sfxProjectile,0.f);
	}
}
