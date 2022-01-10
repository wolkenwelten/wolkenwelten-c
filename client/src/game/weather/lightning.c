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
#include "lightning.h"
#include "../../game/character.h"
#include "../../gui/overlay.h"
#include "../../gfx/effects.h"
#include "../../gfx/particle.h"
#include "../../sfx/sfx.h"
#include "../../tmp/sfx.h"
#include "../../../../common/src/game/weather/lightning.h"

bool lightningQueued = false;
int playerStruckByLightning = 0;

void fxLightningBeam(const vec a, const vec b, int bsize){
	const float mul  = (float)(11 - bsize) / 2.f;
	const float size = (1 << bsize);
	const vec dir    = vecSub(b,a);
	const vec v      = vecMulS(vecNorm(dir),1.f / mul);
	const int start  = (int)vecMag(dir) * mul;
	const u32 color  = 0x40FFFFFF | (((1 << bsize) - 1) << 25);
	const u32 ttl    = MAX(0, (1 << (bsize - 4))) + 256;
	vec t = a;
	for(int i = start;i >= 0; i--){
		newParticle(t.x,t.y,t.z,0,0,0,size,-0.001f,color, ttl);
		t = vecAdd(t, v);
		if(vecMag(vecSub(player->pos, t)) < 4.f){
			playerStruckByLightning = MAX(playerStruckByLightning, bsize);
		}
	}
}

void lightningRecvUpdate(const packet *p){
	const int lx = p->v.u16[0];
	const int ly = p->v.u16[1];
	const int lz = p->v.u16[2];

	const int tx = p->v.u16[3];
	const int ty = p->v.u16[4];
	const int tz = p->v.u16[5];

	const int seed = p->v.u16[6];

	lightningQueued = true;
	const float dist = player == NULL ? 512.f : MIN(vecMag(vecSub(player->pos, vecNew(tx,ty,tz))),vecMag(vecSub(player->pos, vecNew(lx,ly,lz))));
	const float volume = MAX(0.f,MIN(0.8f,((1024 - dist) / 1024.f)));
	sfxPlay(sfxLightning, volume);
	lightningStrike(lx,ly,lz,tx,ty,tz,seed,fxLightningBeam);
	if(playerStruckByLightning){
		if(characterDamage(player,playerStruckByLightning * 2)){
			characterDyingMessage(characterGetBeing(player),0,deathCauseLightning);
		}
	}
	playerStruckByLightning = 0;
}

void lightningDrawOverlay(){
	if(!lightningQueued){return;}
	lightningQueued = false;
	//setOverlayColor(0x80FFFFFF, 100);
}
