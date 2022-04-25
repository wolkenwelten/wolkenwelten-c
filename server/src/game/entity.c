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
#include "entity.h"

#include "../network/server.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/network/messages.h"

void entitySendUpdate(u8 c, uint i){
	entity *ent = &entityList[i];
	packet *p = &packetBuffer;

	p->v.entityUpdate.pos    = ent->pos;
	p->v.entityUpdate.rot    = ent->rot;
	p->v.entityUpdate.vel    = ent->vel;
	p->v.entityUpdate.weight = ent->weight;
	p->v.entityUpdate.flags  = ent->flags;
	p->v.entityUpdate.index  = i;
	p->v.entityUpdate.generation = ent->generation;

	packetQueue(p, msgtEntityUpdate,sizeof(netEntityUpdate) + 4, c);
}

void entitySyncPlayer(u8 c){
	uint start = clients[c].entityUpdateOffset;
	for(int count=0; count < 8; count++){
		uint i = ++clients[c].entityUpdateOffset;
		if(i >= entityMax){
			i = clients[c].entityUpdateOffset = 0;
		}
		entitySendUpdate(c, i);
		if(i == start){break;}
	}
}
