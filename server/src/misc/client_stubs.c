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
/* This file exists so we do not get undefined references when we link in code
 * meant for the client side.
 */
#include "../../../common/src/common.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/src/misc/side.h"

#include <stdio.h>

uint glParticles;
uint   particles;
uint   particleCount;
uint glSparticles;
uint   sparticles;
uint   sparticleCount;

__attribute__((aligned(32))) const float sparticleVV[4][4] = {
	{  0.000001f,0.00004f, 0.000004f, 0.f},
	{ -0.000004f,0.00004f, 0.000001f, 0.f},
	{ -0.000001f,0.00004f,-0.000004f, 0.f},
	{  0.000004f,0.00004f,-0.000001f, 0.f},
};
void particlePosUpdatePortable(){}
void sparticlePosUpdatePortable(){}

void fxRainDrop(const vec pos){
	(void)pos;
}

void fxBlockBreak(const vec pos, u8 b, u8 cause){
	msgMineBlock(pos.x,pos.y,pos.z,b,cause);
}

void ingredientSubstituteAdd(u16 ingredient, u16 substitute){
	(void)ingredient;
	(void)substitute;

	fprintf(stderr,"ingredientSubstituteAdd got called on the serverside\n");
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	(void)ent;
	(void)beamSize;
	(void)damageMultiplier;
	(void)recoilMultiplier;
	(void)hitsLeft;
	(void)shots;
	(void)inaccuracyInc;
	(void)inaccuracyMult;

	fprintf(stderr,"beamblast got called on the serverside\n");
}

void sfxPlay(sfx *b, float volume){
	(void)b;
	(void)volume;
}
void sfxLoop(sfx *b, float volume){
	(void)b;
	(void)volume;
}

void blockTypeGenMeshes(){}
void blockTypeSetTex(u8 b, side cside, u32 tex){
	(void)b;
	(void)cside;
	(void)tex;
}

mesh *meshGet(uint i){
	(void)i;
	return NULL;
}
uint meshIndex(const mesh *m){
	(void)m;
	return 0;
}

void lWidgetMarkI(uint i){
	(void)i;
}
