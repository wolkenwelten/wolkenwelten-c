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
#include "entityGC.h"
#include "../game/entity.h"

void (*entityRootsMarkerChainNext)() = NULL;

static void entityMarker(){
	for(uint i=0;i<entityMax;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(entityList[i].handler == NULL){ continue; }
		lValGCMark(entityList[i].handler);
	}
	if(entityRootsMarkerChainNext){entityRootsMarkerChainNext();}
}

void entityGCInit(){
	entityRootsMarkerChainNext = rootsMarkerChain;
	rootsMarkerChain = entityMarker;
}
