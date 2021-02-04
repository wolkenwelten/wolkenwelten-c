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
#include "animal.h"

#include "savegame.h"
#include "../game/animal.h"
#include "../voxel/chungus.h"

static void *animalSave(const animal *e, void *buf){
	u8    *b = (u8    *)buf;
	u32   *u = (u32   *)buf;
	float *f = (float *)buf;
	if(e->type == 0){return b;}

	b[ 0] = saveTypeAnimal;
	b[ 1] = e->flags;
	b[ 2] = e->type;
	b[ 3] = e->state;

	b[ 4] = e->health;
	b[ 5] = e->hunger;
	b[ 6] = e->pregnancy;
	b[ 7] = e->sleepy;

	b[ 8] = e->age;
	b[ 9] = 0;
	b[10] = 0;
	b[11] = 0;

	f[ 3] = e->pos.x;
	f[ 4] = e->pos.y;
	f[ 5] = e->pos.z;

	f[ 6] = e->vel.x;
	f[ 7] = e->vel.y;
	f[ 8] = e->vel.z;

	f[ 9] = e->rot.yaw;
	f[10] = e->rot.pitch;
	u[11] = e->stateTicks;

	return b+12*4;
}

const void *animalLoad(const void *buf){
	u8 *b         = (u8 *)buf;
	u32 *u        = (u32 *)buf;
	float *f      = (float *)buf;
	if(b[ 2] == 0){return b+12*4;}
	animal *e     = animalNew(vecNewP(&f[3]),b[2],0);
	if(e == NULL){return b+12*4;}

	e->flags      = b[ 1];
	e->type       = b[ 2];
	e->state      = b[ 3];

	e->health     = b[ 4];
	e->hunger     = b[ 5];
	e->pregnancy  = b[ 6];
	e->sleepy     = b[ 7];

	e->age        = b[ 8];

	e->pos        = vecNewP(&f[3]);
	e->vel        = vecNewP(&f[6]);

	e->rot.yaw    = f[ 9];
	e->rot.pitch  = f[10];
	e->rot.roll   = 0.f;

	e->stateTicks = u[11];
	e->target     = 0;

	return b+12*4;
}

void *animalSaveChungus(const chungus *c,void *b){
	if(c == NULL){return b;}
	const u32 cc = c->x | (c->y << 8) | (c->z << 16);
	for(uint i=0;i<animalCount;i++){
		const vec *p = &animalList[i].pos;
		const u32 ac = ((uint)p->x >> 8) | ((uint)p->y & 0xFF00) | (((uint)p->z << 8) & 0xFF0000);
		if(ac != cc){continue;}
		b = animalSave(&animalList[i],b);
	}
	return b;
}
