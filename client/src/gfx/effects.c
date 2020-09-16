#include "effects.h"

#include "../game/blockType.h"
#include "../game/character.h"
#include "../gfx/particle.h"
#include "../gui/gui.h"
#include "../sdl/sfx.h"
#include "../network/chat.h"
#include "../../../common/src/misc/misc.h"
#include "../sdl/input_gamepad.h"


void fxExplosionBomb(float x,float y,float z,float pw){
	sfxPlay(sfxBomb,1.f);

	for(int i=0;i<4096*pw;i++){
		float vx = (rngValf()-0.5f)/6.f*pw*2;
		float vy = (rngValf()-0.5f)/6.f*pw*2;
		float vz = (rngValf()-0.5f)/6.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,vx/-64.f,vy/-64.f,vz/-64.f,64.f,1.f,0xFF44AAFF,64);
	}
	for(int i=0;i<4096*pw;i++){
		float vx = (rngValf()-0.5f)/10.f*pw*2;
		float vy = (rngValf()-0.5f)/10.f*pw*2;
		float vz = (rngValf()-0.5f)/10.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,vx/-78.f,vy/-78.f,vz/-78.f,64.f,1.f,0xFF0099FF,78);
	}
	for(int i=0;i<2048*pw;i++){
		float vx = (rngValf()-0.5f)/16.f*pw*2;
		float vy = (rngValf()-0.5f)/16.f*pw*2;
		float vz = (rngValf()-0.5f)/16.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,vx/-96.f,vy/-96.f,vz/-96.f,64.f,1.f,0xFF0066CC,96);
	}
	for(int i=0;i<2048*pw;i++){
		float vx = (rngValf()-0.5f)/32.f*pw*2;
		float vy = (rngValf()-0.5f)/32.f*pw*2;
		float vz = (rngValf()-0.5f)/32.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,0.f,0.f,0.f,64.f,1.f,0xFF082299,128);
	}

	const float pdx = x - player->x;
	const float pdy = y - player->y;
	const float pdz = z - player->z;
	const float pd  = (pdx*pdx)+(pdy*pdy)+(pdz*pdz);
	if(pd < (pw*8.f)){
		if(characterHP(player,((pw*8.f)-pd) * -3.f * pw)){
			msgSendDyingMessage("got bombed", 65535);
			setOverlayColor(0x00000000,0);
			commitOverlayColor();
		}else{
			setOverlayColor(0xA03020F0,0);
			commitOverlayColor();
		}
	}
}
void fxGrenadeTrail(float x,float y,float z,float pw){
	(void)pw;
	float vx,vy,vz;

	if((rngValR()&3)!=0){return;}
	vx = (rngValf()-0.5f)/32.f;
	vy = (rngValf()-0.5f)/32.f;
	vz = (rngValf()-0.5f)/32.f;
	newParticle(x,y,z,vx,vy,vz,vx/64.f,vy/64.f,vz/64.f,64.f,1.f,0xFF44AAFF,64);
}

void fxExplosionBlaster(float x,float y,float z,float pw){
	for(int i=0;i<512;i++){
		float vx = (rngValf()-0.5f)/8.f*pw;
		float vy = (rngValf()-0.5f)/8.f*pw;
		float vz = (rngValf()-0.5f)/8.f*pw;
		newParticle(x,y,z,vx,vy,vz,vx/-64.f,vy/-64.f,vz/-64.f,64.f,1.f,0xFF964AC0,96);
	}
	for(int i=0;i<512;i++){
		float vx = (rngValf()-0.5f)/12.f*pw;
		float vy = (rngValf()-0.5f)/12.f*pw;
		float vz = (rngValf()-0.5f)/12.f*pw;
		newParticle(x,y,z,vx,vy,vz,vx/-96.f,vy/-96.f,vz/-96.f,64.f,1.f,0xFF7730A0,78);
	}
}
void fxBeamBlaster(float x1,float y1,float z1,float x2,float y2,float z2, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int originatingCharacter){
	(void)recoilMultiplier;
	uint max  = 1<<16;
	float lastDist = 999999.f;
	float minPlayerDist = 100.f;
	vec v,c = vecNew(x2,y2,z2);
	
	uint32_t pac = 0xD0000000 | ((0x50 + rngValM(0x40)) << 16) | ((0x30 + rngValM(0x40)) << 8) | (0xE0 + rngValM(0x1F));
	uint32_t pbc = pac + 0x00202000;
	int plifetime = MIN(128,MAX(48,48 * damageMultiplier));

	sfxPlay(sfxPhaser,MAX(0.5f,damageMultiplier));
	for(max=1<<16;max;--max){
		vec dist = vecNew(x1-c.x,y1-c.y,z1-c.z);
		const float curDist = vecMag(dist);
		if(curDist > lastDist){break;}
		lastDist = curDist;
		v = vecMulS(vecNorm(dist),0.1f/beamSize);
			
		float pvx = (rngValf()-0.5f)/24.f*beamSize;
		float pvy = (rngValf()-0.5f)/24.f*beamSize;
		float pvz = (rngValf()-0.5f)/24.f*beamSize;
		newParticle(c.x+pvx,c.y+pvy,c.z+pvz,pvx/4.f,pvy/4.f,pvz/4.f,-pvx/192.f,-pvy/192.f,-pvz/192.f, 8.f,beamSize*4,pac,plifetime);
		newParticle(c.x+pvx,c.y+pvy,c.z+pvz,pvx/6.f,pvy/6.f,pvz/6.f,-pvx/256.f,-pvy/256.f,-pvz/256.f, 8.f,beamSize*2,pbc,plifetime*2);
		c = vecAdd(c,v);

		const float pd = vecMag(vecSub(c,vecNew(player->x,player->y,player->z)));
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
	(void)hitsLeft;
}

void fxBlockBreak(float x,float y,float z, unsigned char b){
	sfxPlay(sfxTock,1.f);
	for(int i=0;i<128;i++){
		newParticleS(x+rngValf(),y+rngValf(),z+rngValf(),blockTypeGetParticleColor(b),0.f,64);
	}
}
void fxBlockMine(float x,float y,float z, int dmg, unsigned char b){
	int parts=2;
	if(dmg > 1024){
		parts=3;
	}
	for(int i=0;i<parts;i++){
		newParticleS(x+rngValf(),y+rngValf(),z+rngValf(),blockTypeGetParticleColor(b),1.f,64);
	}
}
