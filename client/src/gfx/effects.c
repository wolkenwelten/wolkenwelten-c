#include "effects.h"

#include "../game/blockType.h"
#include "../game/character.h"
#include "../gfx/particle.h"
#include "../gui/gui.h"
#include "../gui/overlay.h"
#include "../network/chat.h"
#include "../sdl/sfx.h"
#include "../sdl/input_gamepad.h"
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
		newParticleV(pos,v,vecMulS(v,1/-64.f),64.f,1.f,0xFF964AC0,96);
	}
	for(int i=0;i<512;i++){
		const vec v = vecMulS(vecRng(),(1.f/12.f)*pw);
		newParticleV(pos,v,vecMulS(v,1/-96.f),64.f,1.f,0xFF7730A0,78);
	}
}
void fxBeamBlaster(const vec pa,const vec pb, float beamSize, float damageMultiplier, float recoilMultiplier, uint hitsLeft, uint originatingCharacter){
	(void)recoilMultiplier;
	(void)hitsLeft;
	uint max            =    1<<16;
	float lastDist      = 999999.f;
	float minPlayerDist =    100.f;
	vec v,c = pb;

	u32 pac  = 0xD0000000 | ((0x50 + rngValM(0x40)) << 16) | ((0x30 + rngValM(0x40)) << 8) | (0xE0 + rngValM(0x1F));
	u32 pbc  = pac + 0x00202000;
	int      ttl  = MIN(128,MAX(48,48 * damageMultiplier));

	sfxPlay(sfxPhaser,MAX(0.5f,damageMultiplier));
	for(max=1<<16;max;--max){
		vec dist = vecSub(pa,c);
		const float curDist = vecMag(dist);
		if(curDist > lastDist){break;}
		lastDist = curDist;
		v = vecMulS(vecNorm(dist),0.1f/beamSize);

		const vec pv = vecMulS(vecRng(),(1.f/64.f)*beamSize);
		newParticleV(vecAdd(c,pv),vecMulS(pv,1/4.f),vecMulS(pv,-1.f/384.f), 8.f,beamSize*4,pac,ttl);
		newParticleV(vecAdd(c,pv),vecMulS(pv,1/6.f),vecMulS(pv,-1.f/512.f), 8.f,beamSize*2,pbc,ttl*2);
		c = vecAdd(c,v);

		const float pd = vecMag(vecSub(c,player->pos));
		if(pd < minPlayerDist){minPlayerDist = pd;}
	}
	if((originatingCharacter != 65535) && (minPlayerDist < 0.5f)){
		if(characterHP(player,(0.6f-minPlayerDist) * -24.f * damageMultiplier)){
			msgSendDyingMessage("Beamblasted", originatingCharacter);
			setOverlayColor(0x00000000,0);
			commitOverlayColor();
		}else{
			setOverlayColor(0xA03020F0,0);
			commitOverlayColor();
		}
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
