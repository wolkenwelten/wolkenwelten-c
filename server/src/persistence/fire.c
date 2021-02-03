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

#include "../game/fire.h"
#include "../voxel/chungus.h"

static void *fireSave(const fire *f, void *buf){
	u8    *b = (u8  *)buf;
	u16   *u = (u16 *)buf;
	i16   *s = (i16 *)buf;

	if(f == NULL){return b;}

	b[0] = 0x04;
	b[1] = 0;

	u[1] = f->x;
	u[2] = f->y;
	u[3] = f->z;

	s[4] = f->strength;
	s[5] = f->blockDmg;
	s[6] = f->oxygen;

	return b+14;
}

const void *fireLoad(const void *buf){
	u8    *b = (u8  *)buf;
	u16   *u = (u16 *)buf;
	i16   *s = (i16 *)buf;

	fireNewF(u[1],u[2],u[3],s[4],s[5],s[6]);
	return b+14;
}

void *fireSaveChungus(const chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(uint i=0;i<fireCount;i++){
		if(c->x != (fireList[i].x >> 8)){continue;}
		if(c->y != (fireList[i].y >> 8)){continue;}
		if(c->z != (fireList[i].z >> 8)){continue;}
		buf = fireSave(&fireList[i],buf);
	}
	return buf;
}
