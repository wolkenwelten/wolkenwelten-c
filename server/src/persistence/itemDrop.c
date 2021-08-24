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
#include "itemDrop.h"

#include "savegame.h"
#include "../game/itemDrop.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/game/item.h"

static void *itemDropSave(const itemDrop *i, void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	if(i      == NULL){return b;}
	if(i->ent == NULL){return b;}

	b[0] = saveTypeItemDrop;
	b[1] = 0;

	s[1] = i->itm.ID;
	s[2] = i->itm.amount;
	s[3] = 0;

	f[2] = i->ent->pos.x;
	f[3] = i->ent->pos.y;
	f[4] = i->ent->pos.z;
	f[5] = i->ent->vel.x;
	f[6] = i->ent->vel.y;
	f[7] = i->ent->vel.z;

	return b+32;
}

const void *itemDropLoad(const void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	itemDrop *id = itemDropNew();
	if(id == NULL){return b+32;}
	id->itm.ID     = s[1];
	id->itm.amount = s[2];
	id->player     = -1;

	id->ent = entityNew(vecNewP(&f[2]),vecZero(), itemGetWeight(&id->itm));
	if(id->ent == NULL){return b+32;}
	id->ent->vel = vecNewP(&f[5]);

	return b+32;
}

void *itemDropSaveChungus(const chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(uint i=0;i<itemDropCount;i++){
		if(itemIsEmpty(&itemDropList[i].itm))   {continue;}
		if(itemDropList[i].ent == NULL)         {continue;}
		if(itemDropList[i].ent->curChungus != c){continue;}
		buf = itemDropSave(&itemDropList[i],buf);
	}
	return buf;
}
