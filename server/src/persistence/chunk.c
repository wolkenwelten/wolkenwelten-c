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

#include <string.h>

void *chunkSave(chunk *c, void *rbuf){
	u8 *buf = rbuf;
	if((c->clientsUpdated & ((u64)1 << 31)) != 0){return buf;}
	buf[0] = saveTypeChunk;
	buf[1] = (c->x >> 4)&0xF;
	buf[2] = (c->y >> 4)&0xF;
	buf[3] = (c->z >> 4)&0xF;
	memcpy(buf+4,c->data,16*16*16);
	return buf+4100;
}

const void *chunkLoad(chungus *c, const void *rbuf){
	const u8 *buf = rbuf;

	int cx = buf[1] & 0xF;
	int cy = buf[2] & 0xF;
	int cz = buf[3] & 0xF;

	chunk *chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew(c->x+(cx<<4),c->y+(cy<<4),c->z+(cz<<4));
	}
	chnk->clientsUpdated = 0;
	memcpy(chnk->data,&buf[4],4096);

	return buf+4100;
}
