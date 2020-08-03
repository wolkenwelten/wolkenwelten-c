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

	for(int i=0;i<512*pw;i++){
		float vx = (rngValf()-0.5f)/6.f*pw*2;
		float vy = (rngValf()-0.5f)/6.f*pw*2;
		float vz = (rngValf()-0.5f)/6.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,vx/-64.f,vy/-64.f,vz/-64.f,0xFF44AAFF,64);
	}
	for(int i=0;i<512*pw;i++){
		float vx = (rngValf()-0.5f)/10.f*pw*2;
		float vy = (rngValf()-0.5f)/10.f*pw*2;
		float vz = (rngValf()-0.5f)/10.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,vx/-78.f,vy/-78.f,vz/-78.f,0xFF0099FF,78);
	}
	for(int i=0;i<256*pw;i++){
		float vx = (rngValf()-0.5f)/16.f*pw*2;
		float vy = (rngValf()-0.5f)/16.f*pw*2;
		float vz = (rngValf()-0.5f)/16.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,vx/-96.f,vy/-96.f,vz/-96.f,0xFF0066CC,96);
	}
	for(int i=0;i<256*pw;i++){
		float vx = (rngValf()-0.5f)/32.f*pw*2;
		float vy = (rngValf()-0.5f)/32.f*pw*2;
		float vz = (rngValf()-0.5f)/32.f*pw*2;
		newParticle(x,y,z,vx,vy,vz,0.f,0.f,0.f,0xFF082299,128);
	}

	const float pdx = x - player->x;
	const float pdy = y - player->y;
	const float pdz = z - player->z;
	const float pd  = (pdx*pdx)+(pdy*pdy)+(pdz*pdz);
	if(pd < (pw*4.f)){
		if(characterHP(player,((pw*4.f)-pd) * -2.f * pw)){
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
	newParticle(x,y,z,vx,vy,vz,vx/64.f,vy/64.f,vz/64.f,0xFF44AAFF,64);
}

void fxExplosionBlaster(float x,float y,float z,float pw){
	for(int i=0;i<128;i++){
		float vx = (rngValf()-0.5f)/8.f*pw;
		float vy = (rngValf()-0.5f)/8.f*pw;
		float vz = (rngValf()-0.5f)/8.f*pw;
		newParticle(x,y,z,vx,vy,vz,vx/-64.f,vy/-64.f,vz/-64.f,0xFF3A1F90,64);
	}
	for(int i=0;i<128;i++){
		float vx = (rngValf()-0.5f)/12.f*pw;
		float vy = (rngValf()-0.5f)/12.f*pw;
		float vz = (rngValf()-0.5f)/12.f*pw;
		newParticle(x,y,z,vx,vy,vz,vx/-96.f,vy/-96.f,vz/-96.f,0xFF7730A0,128);
	}
}
void fxBeamBlaster(float x1,float y1,float z1,float x2,float y2,float z2, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int originatingCharacter){
	int steps = 0;
	float minPlayerDist = 100.f;
	float cx = x1;
	float cy = y1;
	float cz = z1;

	float dx = x2-x1;
	float dy = y2-y1;
	float dz = z2-z1;
	float dm = fabsf(dx);
	if(fabsf(dy) > dm){dm = fabsf(dy);}
	if(fabsf(dz) > dm){dm = fabsf(dz);}

	float vx = (dx / dm)/8.f;
	float vy = (dy / dm)/8.f;
	float vz = (dz / dm)/8.f;

	if(fabsf(vx) > fabsf(vy)){
		if(fabsf(vx) > fabsf(vz)){
			steps = (x2 - x1) / vx;
		}else{
			steps = (z2 - z1) / vz;
		}
	}else{
		if(fabsf(vy) > fabsf(vz)){
			steps = (y2 - y1) / vy;
		}else{
			steps = (z2 - z1) / vz;
		}
	}
	sfxPlay(sfxPhaser,recoilMultiplier);
	for(;steps > 0;steps--){
		float pvx = (rngValf()-0.5f)/8.f*beamSize;
		float pvy = (rngValf()-0.5f)/8.f*beamSize;
		float pvz = (rngValf()-0.5f)/8.f*beamSize;
		newParticle(cx+pvx,cy+pvy,cz+pvz,pvx/4.f,pvy/4.f,pvz/4.f,-pvx/192.f,-pvy/192.f,-pvz/192.f,0xFF8F56FF,128);
		newParticle(cx+pvx,cy+pvy,cz+pvz,pvx/6.f,pvy/6.f,pvz/6.f,-pvx/256.f,-pvy/256.f,-pvz/256.f,0xFFAF76FF,178);
		//newParticle(cx+pvx,cy+pvy,cz+pvz,pvx/8.f,pvy/8.f,pvz/8.f,-pvx/512.f,-pvy/512.f,-pvz/512.f,0x6FBF26FF,192);
		cx += vx;
		cy += vy;
		cz += vz;

		const float pdx = cx - player->x;
		const float pdy = cy - player->y;
		const float pdz = cz - player->z;
		const float pd  = (pdx*pdx)+(pdy*pdy)+(pdz*pdz);
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
