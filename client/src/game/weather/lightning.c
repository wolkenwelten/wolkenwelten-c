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
#include "weather.h"
#include "../../game/character/character.h"
#include "../../gui/overlay.h"
#include "../../gfx/effects.h"
#include "../../gfx/particle.h"
#include "../../sfx/sfx.h"
#include "../../tmp/sfx.h"
#include "../../../../common/src/game/weather/lightning.h"
#include "../../../../common/src/misc/line.h"
#include "../../../../common/src/misc/profiling.h"
#include "../../voxel/bigchungus.h"

bool lightningQueued = false;
int playerStruckByLightning = 0;

int fireBeamSize = 0;
void lightningFireBeamCB(int x, int y, int z){
	if(worldTryB(x,y,z)){
		worldSetFire(x,y,z,fireBeamSize);
	}
}

int lightningBlockCountVal = 0;
void lightningBlockCountCB(int x, int y, int z){
	if(worldTryB(x,y,z)){lightningBlockCountVal++;}
}
void lightningBlockCount(const vec a, const vec b, int bsize){
	(void)bsize;
	lineFromTo(a.x, a.y, a.z, b.x, b.y, b.z, lightningBlockCountCB);
}

void lightningFireBeam(const vec a, const vec b, int bsize){
	fireBeamSize = 1 << (bsize + 2);
	lineFromTo(a.x, a.y, a.z, b.x, b.y, b.z, lightningFireBeamCB);
}

void tryLightningChungus(const chungus *chng){
	const int cx = ((int)chng->x << 8);
	const int cy = ((int)chng->y << 8);
	const int cz = ((int)chng->z << 8);
	if(!(chng->y & 1)){return;}

	const int lx = cx + rngValA(255);
	const int lz = cz + rngValA(255);
	const int ly = cy + 256 + 32;
	const vec lv = vecNew(lx,ly,lz);
	if(!isInClouds(lv)){return;}

	const int tx = cx + rngValA(255);
	const int tz = cz + rngValA(255);
	int ty;
	for(ty = cy + rngValA(255); worldTryB(tx,ty,tz); ty++){}
	if(!worldTryB(tx,--ty,tz)){return;}
	const u16 seed = rngValA((1<<16)-1);

	lightningBlockCountVal = 0;
	lightningStrike(lx,ly,lz,tx,ty,tz,seed,lightningBlockCount);
	if(lightningBlockCountVal > (stormIntensity / 2)){
		return;
	}
	lightningStrike(lx,ly,lz,tx,ty,tz,seed,lightningFireBeam);
}

void tryLightning(){
	static int calls = 0;
	PROFILE_START();

	for(uint i=++calls & 0x1F;i < chungusCount; i += 0x20){
		const chungus *chng = &chungusList[i];
		if(chng->nextFree != NULL){continue;}
		//if(rngValA(0x1F)){continue;}
		tryLightningChungus(chng);
	}

	PROFILE_STOP();
}

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

/*
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
*/

void lightningDrawOverlay(){
	if(!lightningQueued){return;}
	lightningQueued = false;
	//setOverlayColor(0x80FFFFFF, 100);
}
