#include "water.h"

#include "../game/character.h"
#include "../gfx/particle.h"
#include "../gui/overlay.h"
#include "../network/chat.h"
#include "../sdl/sfx.h"
#include "../../../common/src/network/messages.h"

#include <string.h>

void waterNew(u16 x, u16 y, u16 z, i16 strength){
	msgWaterUpdate(-1,0,0,x,y,z,strength);
}

static void waterDraw(const water *w){
	const vec spos = vecNew(w->x,w->y,w->z);
	const float size = (float)(w->amount * 0.01f);
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.00008f,0.f),size, size*0.5f,0xFF60C8FF, 96);
	if(w->amount <  64){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.0001f,0.f),size*0.7f, size*0.65f,0xFF5098FF, 128);
	if(w->amount < 128){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.0001f,0.f),size*0.5f, size*0.75f,0xFF1F38EF, 156);
	if(w->amount < 256){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecNew(0.f,0.0001f,0.f),size*0.5f, size*0.75f,0xFF1F38EF, 178);
	if(w->amount < 512){return;}
	if((rngValR()&0xF) != 0){return;}
	u32 c = 0xFF101820 | (rngValR()&0x0003070F);
	const vec vv = vecNew(rngValf()*0.00002f,0.00005f+rngValf()*0.00002f,rngValf()*0.00002f);
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vv,size*0.01f, size*0.2f,c,2048);
}

void waterDrawAll(){
	for(uint i=0;i<waterCount;i++){
		waterDraw(&waterList[i]);
	}
}

void waterRecvUpdate(uint c, const packet *p){
	(void)c;
	const uint i     = p->v.u16[0];
	const uint count = p->v.u16[1];
	if(count > waterCount){
		memset(&waterList[waterCount],0,sizeof(water) * (count-waterCount));
	}
	waterCount = count;
	water *w = &waterList[i];
	w->x = p->v.u16[2];
	w->y = p->v.u16[3];
	w->z = p->v.u16[4];
	w->amount = p->v.i16[5];
}

void waterCheckPlayerBurn(uint off){
	for(uint i=off&0x7F;i<waterCount;i+=0x80){
		const water *w  = &waterList[i];
		const vec wpos  = vecNew(w->x,w->y,w->z);
		const vec  dist = vecSub(player->pos,wpos);
		const float  dd = vecDot(dist,dist);
		const float fdd = MIN(9.f,(waterList[i].amount * 0.01f) * (waterList[i].amount * 0.01f));
		if(dd < fdd){
			sfxPlay(sfxUngh,1.f);
			setOverlayColor(0xA03020F0,0);
			if(characterHP(player,-1)){
				msgSendDyingMessage("burned", 65535);
				setOverlayColor(0xFF000000,0);
				commitOverlayColor();
			}
		}
	}
}
