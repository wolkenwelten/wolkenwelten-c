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
#include "chunkOverlay.h"

#include <stdlib.h>
#include <string.h>

chunkOverlay *chunkOverlayFirstFree = NULL;

#define ALLOCATION_BLOCK_SIZE (1<<21)

void chunkOverlayAllocateBlock(){
	void *block = malloc(ALLOCATION_BLOCK_SIZE);
	chunkOverlay *d = (chunkOverlay *)block;
	for(uint i=0;i < ALLOCATION_BLOCK_SIZE / sizeof(chunkOverlay); i++){
		d[i].nextFree = chunkOverlayFirstFree;
		chunkOverlayFirstFree = &d[i];
	}
}

chunkOverlay *chunkOverlayAllocate(){
	if(chunkOverlayFirstFree == NULL){chunkOverlayAllocateBlock();}
	chunkOverlay *ret = chunkOverlayFirstFree;
	chunkOverlayFirstFree = ret->nextFree;
	memset(ret,0,sizeof(chunkOverlay));
	return ret;
}

void chunkOverlayFree(chunkOverlay *d){
	d->nextFree = chunkOverlayFirstFree;
	chunkOverlayFirstFree = d;
}
