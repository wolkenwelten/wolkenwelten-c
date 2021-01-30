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
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

beingList *beingListGet(u16 x, u16 y, u16 z){
	chunk *chnk = worldGetChunk(x,y,z);
	if(chnk != NULL){return &chnk->bl;}
	chungus *chng = worldTryChungus(x>>8,y>>8,z>>8);
	if(chng != NULL){return &chng->bl;}
	return NULL;
}
