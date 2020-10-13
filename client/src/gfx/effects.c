#include "effects.h"

#include "../game/blockType.h"
#include "../game/character.h"
#include "../gfx/particle.h"
#include "../gui/overlay.h"
#include "../network/chat.h"
#include "../sdl/sfx.h"
#include "../../../common/src/misc/misc.h"


void fxExplosionBomb(const vec pos,float pw){
	sfxPlay(sfxBomb,1.f);

	for(int i=0;i<4096*pw;i++){
		const vec v  = vecMulS(vecRng(),0.15f*pw);
		newParticleV(pos,v,vecMulS(v,1/-64.f),64.f,1.f,0xFF44AAFF,64);
	}
	for(int i=0;i<4096*pw;i++){
		const vec v  = vecMulS(vecRng(),0.1f*pw);
		newParticleV(pos,v,vecMulS(v,1/-78.f),64.f,1.f,0xFF0099FF,78);
	}
	for(int i=0;i<2048*pw;i++){
		const vec v  = vecMulS(vecRng(),0.06f*pw);
		newParticleV(pos,v,vecMulS(v,1/-96.f),64.f,1.f,0xFF0066CC,96);
	}
	for(int i=0;i<2048*pw;i++){
		const vec v  = vecMulS(vecRng(),0.03f*pw);
		newParticleV(pos,v,vecZero(),64.f,1.f,0xFF082299,128);
	}

	const float pd  = vecMag(vecSub(pos,player->pos));
	const float max = 16*pw*pw;
	if(pd < max){
		if(characterDamage(player,((max-pd)/max)*pw*4.f)){
			msgSendDyingMessage("got bombed", 65535);
			setOverlayColor(0x00000000,0);
			commitOverlayColor();
		}else{
			setOverlayColor(0xA03020F0,0);
			commitOverlayColor();
		}
	}
}
void fxGrenadeTrail(const vec pos,float pw){
	(void)pw;
	if((rngValR()&3)!=0){return;}
	const vec v = vecMulS(vecRng(),0.008f);
	newParticleV(pos,v,vecZero(),16.f,2.f,0xFF44AAFF,64);
}

void fxExplosionBlaster(const vec pos,float pw){
	for(int i=0;i<512;i++){
		const vec v = vecMulS(vecRng(),(1.f/8.f)*pw);
		newParticleV(pos,v,vecMulS(v,1/-64.f),32.f,2.f,0xFF964AC0,156);
	}
	for(int i=0;i<512;i++){
		const vec v = vecMulS(vecRng(),(1.f/12.f)*pw);
		newParticleV(pos,v,vecMulS(v,1/-96.f),16.f,4.f,0xFF7730A0,128);
	}
}
void fxBeamBlaster(const vec pa,const vec pb, float beamSize, float damageMultiplier){
	float lastDist = 999999.f;
	vec v,c = pb;

	u32 pac  = 0xD0000000 | ((0x50 + rngValM(0x40)) << 16) | ((0x30 + rngValM(0x40)) << 8) | (0xE0 + rngValM(0x1F));
	u32 pbc  = pac + 0x00202000;
	int ttl  = MIN(1922,MAX(64,64 * damageMultiplier));
	//int ttl = 128;
	sfxPlay(sfxPhaser,MIN(0.5f,MAX(0.2f,damageMultiplier)));

	for(uint max=1<<12;max;--max){
		vec dist = vecSub(pa,c);
		const float curDist = vecMag(dist);
		if(curDist > lastDist){break;}
		lastDist = curDist;
		v = vecMulS(vecNorm(dist),beamSize*8);

		for(int i=0;i<32;i++){
			const vec pv = vecMulS(vecRng(),1/(beamSize*32));
			const vec po = vecMulS(v,i/32.f);
			newParticleV(vecAddT(c,pv,po),vecMulS(pv,1/ 8.f),vecMulS(pv,-1.f/384.f),beamSize*12,beamSize  ,pac,ttl  );
			newParticleV(vecAddT(c,pv,po),vecMulS(pv,1/12.f),vecMulS(pv,-1.f/512.f),beamSize*12,beamSize/2,pbc,ttl*2);
		}
		c = vecAdd(c,v);
	}
}

void fxBlockBreak(const vec pos, unsigned char b){
	sfxPlay(sfxTock,1.f);
	for(int i=0;i<128;i++){
		const vec p = vecAdd(pos,vecRngAbs());
		newParticleS(p.x,p.y,p.z,blockTypeGetParticleColor(b),0.f,64);
	}
}
void fxBlockMine(const vec pos, int dmg, unsigned char b){
	int parts=2;
	if(dmg > 1024){
		parts=3;
	}
	for(int i=0;i<parts;i++){
		const vec p = vecAdd(pos,vecRngAbs());
		newParticleS(p.x,p.y,p.z,blockTypeGetParticleColor(b),1.f,64);
	}
}

void fxBleeding(const vec pos, being victim, i16 dmg, u16 cause){
	(void)victim;
	(void)dmg;
	(void)cause;
	const float d = vecMag(vecSub(player->pos,pos));
	if(d < 128.f){
		const float vol = (128.f-d)/128.f;
		sfxPlay(sfxImpact,vol);
		sfxPlay(sfxUngh,vol);
	}
	for(int i=dmg*64;i>0;i--){
		const vec v  = vecMulS(vecRng(),0.06f);
		newParticleV(pos,v,vecMulS(v,1/-64.f),64.f,1.f,0xFF44AAFF,64);
	}
}
