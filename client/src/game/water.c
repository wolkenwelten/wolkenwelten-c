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
	const float size = (float)MIN(192.f,(w->amount * 0.1f));
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecZero(),size, size*-0.0005f,0xFFFFC860, 256);
	if(w->amount <  64){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecZero(),size*.9f, size*-0.00065f,0xFFFF9850, 384);
	if(w->amount < 128){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecMulS(vecRng(),0.0001f ), vecZero(),size*.8f, size*-0.00075f,0xFFEF381F, 512);
	if(w->amount < 256){return;}
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
