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

#include "animal.h"

#include "../misc/options.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static void animalServerSync(u8 c, u16 i){
	if(i >= countof(animalList)){return;}
	const animal *e = &animalList[i];
	if(!chungusIsSubscribed(worldTryChungusV(e->pos),c)){
		return animalSyncInactive(c,i);
	}
	return animalSync(c,i);
}

void animalSyncPlayer(u8 c){
	if(animalListMax == 0){
		animalEmptySync(c);
		return;
	}

	const u64 mask = 1 << c;
	int count = clients[c].animalUpdateWindowSize;
	for(uint tries = 256;count >= 0;clients[c].animalUpdateOffset++){
		if(--tries == 0){break;}
		const uint i = clients[c].animalUpdateOffset;
		if(i >= animalListMax){clients[c].animalUpdateOffset = 0;}
		if(animalList[i].clientPriorization & mask){continue;}
		animalServerSync(c,i);
		count--;
	}

	count = clients[c].animalUpdateWindowSize;
	for(uint tries=2048;count >= 0;clients[c].animalPriorityUpdateOffset++){
		if(--tries == 0){break;}
		const uint i = clients[c].animalPriorityUpdateOffset;
		if(i >= animalListMax){clients[c].animalPriorityUpdateOffset = 0;}
		if(!(animalList[i].clientPriorization & mask)){continue;}
		animalServerSync(c,i);
		count--;
	}

	if(getClientLatency(c) < 50){
		clients[c].animalUpdateWindowSize += 1;
	}else{
		clients[c].animalUpdateWindowSize /= 2;
	}
	clients[c].animalUpdateWindowSize = MAX(4,MIN(12,clients[c].animalUpdateWindowSize));
}

void animalUpdatePriorities(u8 c){
	const u64 prio = 1 << c;
	const u64 mask = ~(prio);
	uint countPrio = 0;
	if(clients[c].state)     {return;}
	if(clients[c].c == NULL) {return;}
	const vec cpos = clients[c].c->pos;
	for(uint i=0;i<animalListMax;i++){
		const float d = vecMag(vecSub(animalList[i].pos,cpos));
		if(d < 78.f){
			animalList[i].clientPriorization |= prio;
			countPrio++;
		}else{
			animalList[i].clientPriorization &= mask;
		}
	}
}
