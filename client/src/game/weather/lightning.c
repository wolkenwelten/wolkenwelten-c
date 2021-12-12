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
#include "../../gui/overlay.h"
#include "../../gfx/effects.h"
#include "../../gfx/particle.h"
#include "../../sdl/sfx.h"
#include "../../tmp/sfx.h"

bool lightningQueued = false;

void fxLightningBeam(const vec a, const vec b, int size){
	vec t = a;
	const vec dir = vecSub(b,a);
	const vec v = vecMulS(vecNorm(dir),0.5f);
	const int start = (int)vecMag(dir) * 2;
	for(int i = start;i > 0; i--){
		newParticle(t.x,t.y,t.z,0,0,0,size,-0.1f,0xFFFFFF | ((size & 0x3F3) << 14),(size >> 5) + 64);
		t = vecAdd(t, v);
	}
}

void fxLightningStrike(const vec a, const vec b, uint stepsLeft, int size){
	const vec t = vecAdd(vecAdd(a, vecMulS(vecSub(b,a),0.5f)), vecMulS(vecRng(),stepsLeft * 2));
	if((size > 512.f) && (rngValA(7) == 0)){
		vec branch = vecAdd(vecAdd(vecAdd(a, vecMulS(vecSub(t,a),0.5f)), vecMulS(vecRng(),stepsLeft * 9)), vecNew(0.f, -(vecMag(vecSub(b,a)) / 2), 0.f));
		fxLightningStrike(a, branch, 4, size/2.f);
	}
	if(stepsLeft > 0){
		fxLightningStrike(a,t,stepsLeft-1, size);
		fxLightningStrike(t,b,stepsLeft-1, size);
	}else{
		fxLightningBeam(a,b,size);
	}
}

void lightningRecvUpdate(const packet *p){
	const int lx = p->v.u16[0];
	const int ly = p->v.u16[1];
	const int lz = p->v.u16[2];

	const int tx = p->v.u16[3];
	const int ty = p->v.u16[4];
	const int tz = p->v.u16[5];

	lightningQueued = true;
	sfxPlay(sfxImpact, 0.3f);
	fxLightningStrike(vecNew(lx,ly,lz),vecNew(tx,ty,tz), 6, 1024);
}

void lightningDrawOverlay(){
	if(!lightningQueued){return;}
	lightningQueued = false;
	//setOverlayColor(0x80FFFFFF, 100);
}
