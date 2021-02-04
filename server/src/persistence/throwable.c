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
#include "throwable.h"

#include "savegame.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../game/throwable.h"
#include "../../../common/src/game/item.h"

static void *throwableSave(const throwable *t, void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	if(t      == NULL){return b;}
	if(t->ent == NULL){return b;}

	b[ 0] = saveTypeThrowable;
	b[ 1] = t->flags;
	b[ 2] = t->damage;
	b[ 3] = 0;

	s[ 2] = t->itm.ID;
	s[ 3] = t->itm.amount;
	s[ 4] = t->counter;
	s[ 5] = 0;

	f[ 3] = t->ent->pos.x;
	f[ 4] = t->ent->pos.y;
	f[ 5] = t->ent->pos.z;
	f[ 6] = t->ent->vel.x;
	f[ 7] = t->ent->vel.y;
	f[ 8] = t->ent->vel.z;
	f[ 9] = t->ent->rot.yaw;
	f[10] = t->ent->rot.pitch;

	return b+44;
}

const void *throwableLoad(const void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	throwable *t = throwableAlloc();
	if(t == NULL){return b+44;}
	t->flags      = b[1];
	t->damage     = b[2];

	t->itm.ID     = s[2];
	t->itm.amount = s[3];
	t->counter    = s[4];

	t->ent = entityNew(vecNewP(&f[3]),vecNew(f[9],f[10],0));
	if(t->ent == NULL){return b+44;}
	t->ent->vel = vecNewP(&f[6]);

	return b+44;
}

void *throwableSaveChungus(const chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(uint i=0;i<throwableCount;i++){
		if(itemIsEmpty(&throwableList[i].itm))   {continue;}
		if(throwableList[i].ent == NULL)         {continue;}
		if(throwableList[i].ent->curChungus != c){continue;}
		buf = throwableSave(&throwableList[i],buf);
	}
	return buf;
}
