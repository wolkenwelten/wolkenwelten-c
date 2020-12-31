#include "water.h"

#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
#include "../gui/overlay.h"
#include "../network/chat.h"
#include "../sdl/sfx.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

#include <string.h>

int waterNew(u16 x, u16 y, u16 z, i16 amount){
	msgWaterUpdate(-1,0,0,x,y,z,amount);
	return amount;
}

static void waterDraw(const water *w){
	if(abs(w->x-(int)player->pos.x) > renderDistance){return;}
	if(abs(w->y-(int)player->pos.y) > renderDistance){return;}
	if(abs(w->z-(int)player->pos.z) > renderDistance){return;}
	const vec spos = vecNew(w->x,w->y,w->z);
	const float size = (float)MIN(256.f,(w->amount * 0.1f));

	if(w->bb == 0){
		float ma = w->amount / 65536.f;
		for(int i=0;i<2;i++){
			const vec off = vecRngAbs();
			if(fabsf(off.x - 0.5f) > ma){continue;}
			if(fabsf(off.z - 0.5f) > ma){continue;}
			newParticleV(vecAdd(spos,off), vecMulS(vecRng(),0.0001f ), size*.8f, size*-0.001f,0xFFD04020|rngValA(0x001F0F0F), 256);
		}
	}else{
		float my = w->amount / 32768.f;
		const vec off = vecRngAbs();
		if(off.y <= my){
			newParticleV(vecAdd(spos,off), vecMulS(vecRng(),0.0001f ), size*.8f, size*-0.001f,0xFFB05020|rngValA(0x003F1F0F), 256);
		}
	}
}

void waterDrawAll(){
	for(uint i=0;i<waterCount;i++){
		if(waterList[i].b != 0){continue;}
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
	w->b  = worldTryB(w->x,w->y,w->z);
	w->bb = worldTryB(w->x,w->y-1,w->z);
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
