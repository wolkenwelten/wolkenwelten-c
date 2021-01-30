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

#include "fire.h"

#include "../main.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
#include "../gui/overlay.h"
#include "../network/chat.h"
#include "../sdl/sdl.h"
#include "../sdl/sfx.h"
#include "../../../common/src/network/messages.h"

#include <string.h>

void fireNew(u16 x, u16 y, u16 z, i16 strength){
	msgFireUpdate(-1,0,0,x,y,z,strength);
}

static void fireDraw(const fire *f){
	if(abs(f->x-(int)player->pos.x) > renderDistance){return;}
	if(abs(f->y-(int)player->pos.y) > renderDistance){return;}
	if(abs(f->z-(int)player->pos.z) > renderDistance){return;}
	const vec spos = vecNew(f->x,f->y,f->z);
	const float size = (float)(f->strength * 0.01f);
	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size, size*0.5f,0xFF60C8FF, 96);
	if(f->strength <  64){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size*0.7f, size*0.65f,0xFF5098FF, 128);
	if(f->strength < 128){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size*0.6f, size*0.75f,0xFF1F38EF, 156);
	if(f->strength < 256){return;}
	newParticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.01f,0),vecMulS(vecRng(),0.0001f)), size*0.5f, size*0.75f,0xFF1F38EF, 178);
}

static void fireDrawSmoke(const fire *f){
	if(f->strength < 256){return;}
	if(abs(f->x-(int)player->pos.x) > renderDistance){return;}
	if(abs(f->y-(int)player->pos.y) > renderDistance){return;}
	if(abs(f->z-(int)player->pos.z) > renderDistance){return;}
	const vec spos = vecNew(f->x,f->y,f->z);
	const float size = (float)(f->strength * 0.01f);
	u32 c = 0x00101820 | (rngValR()&0x0003070F);
	newSparticleV(vecAdd(spos,vecRngAbs()), vecAdd(vecNew(0,0.001f,0),vecMulS(vecRng(),0.0001f)), size*0.01f, size*0.2f,c,2048);
}

void fireDrawAll(){
	static uint calls = 0;
	static  int lastTick = 0;
	int curTick;

	if(lastTick == 0){lastTick = getTicks();}
	curTick = getTicks();
	for(;lastTick < curTick;lastTick+=msPerTick){
		for(uint i=calls&0x1F;i<fireCount;i+=0x20){
			fireDraw(&fireList[i]);
		}
		for(uint i=calls&0xFF;i<fireCount;i+=0xFF){
			fireDrawSmoke(&fireList[i]);
		}
		calls++;
	}
}

void fireRecvUpdate(uint c, const packet *p){
	(void)c;
	const uint i     = p->v.u16[0];
	const uint count = p->v.u16[1];
	if(count > fireCount){
		memset(&fireList[fireCount],0,sizeof(fire) * (count-fireCount));
	}
	fireCount = count;
	fire *f = &fireList[i];
	f->x = p->v.u16[2];
	f->y = p->v.u16[3];
	f->z = p->v.u16[4];
	f->strength = p->v.i16[5];
}

void fireCheckPlayerBurn(uint off){
	for(uint i=off&0x7F;i<fireCount;i+=0x80){
		const fire *f   = &fireList[i];
		const vec fpos  = vecNew(f->x,f->y,f->z);
		const vec  dist = vecSub(player->pos,fpos);
		const float  dd = vecDot(dist,dist);
		const float fdd = MIN(9.f,(fireList[i].strength * 0.01f) * (fireList[i].strength * 0.01f));
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
