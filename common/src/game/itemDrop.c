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

#include "../game/character.h"
#include "../game/entity.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>

itemDrop itemDropList[1<<14];
uint     itemDropCount = 0;
int      itemDropFirstFree = -1;

itemDrop *itemDropGetByBeing(being b){
	if(beingType(b) != BEING_ITEMDROP){return NULL;}
	uint i = beingID(b);
	if(i >= itemDropCount){return NULL;}
	return &itemDropList[i];
}

being itemDropGetBeing(const itemDrop *id){
	if(id == NULL){return 0;}
	return beingItemDrop(id - itemDropList);
}
