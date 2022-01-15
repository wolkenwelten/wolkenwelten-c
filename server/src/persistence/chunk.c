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
#include "chunk.h"

#include "savegame.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/chunkOverlay.h"

#include <string.h>
#include <stdio.h>

void *chunkSave(chunk *c, void *rbuf, saveType t){
	u8 *buf = rbuf;
	if(c->block == NULL){return buf;}
	if((c->clientsUpdated & ((u64)1 << 31)) != 0){return buf;}
	buf[0] = t;
	buf[1] = (c->x >> 4)&0xF;
	buf[2] = (c->y >> 4)&0xF;
	buf[3] = (c->z >> 4)&0xF;
	void *src = NULL;
	switch(t){
	default:
		return buf;
	case saveTypeChunkBlockData:
		src = c->block ? c->block->data : NULL;
		break;
	case saveTypeChunkFluidData:
		src = c->fluid ? c->fluid->data : NULL;
		break;
	case saveTypeChunkFireData:
		src = c->fire ? c->fire->data : NULL;
		break;
	}
	if(src == NULL){return buf;}
	memcpy(buf+4, src, 16*16*16);
	return buf+4100;
}

const void *chunkLoad(chungus *c, const void *rbuf){
	const u8 *buf = rbuf;

	saveType t = buf[0];
	int cx = buf[1] & 0xF;
	int cy = buf[2] & 0xF;
	int cz = buf[3] & 0xF;

	chunk *chnk = &c->chunks[cx][cy][cz];
	chnk->clientsUpdated = 0;
	void *dest = NULL;
	switch(t){
		default:
		return buf+4100;
	case saveTypeChunkBlockData:
		if(chnk->block == NULL){chnk->block = chunkOverlayAllocate();}
		dest = chnk->block->data;
		break;
	case saveTypeChunkFluidData:
		if(chnk->fluid == NULL){chnk->fluid = chunkOverlayAllocate();}
		dest = chnk->fluid->data;
		break;
	case saveTypeChunkFireData:
		if(chnk->fire == NULL){chnk->fire = chunkOverlayAllocate();}
		dest = chnk->fire->data;
		break;
	}
	memcpy(dest, &buf[4], 4096);

	return buf+4100;
}
