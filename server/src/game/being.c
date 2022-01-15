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

#include "being.h"
#include "../game/animal.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/projectile.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

beingList *beingListGet(u16 x, u16 y, u16 z){
	chunk *chnk = worldTryChunk(x,y,z);
	if(chnk != NULL){return &chnk->bl;}
	chungus *chng = worldTryChungus(x>>8,y>>8,z>>8);
	if(chng != NULL){return &chng->bl;}
	return NULL;
}

void beingSync(u8 c, being b){
	switch(beingType(b)){
	default:
		return;
	case BEING_ANIMAL:
		animalSync(c,beingID(b));
		return;
	case BEING_PROJECTILE:
		projectileSendUpdate(c,beingID(b));
		return;
	case BEING_ITEMDROP:
		itemDropUpdateMsg(c,beingID(b));
		return;
	}
}

void beingListSync(u8 c, const beingList *bl){
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		for(uint i=0; i<countof(ble->v); i++){
			if(ble->v[i] == 0){break;}
			beingSync(c,ble->v[i]);
		}
	}
}
