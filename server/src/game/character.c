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

#include "character.h"

#include "../game/being.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>

character *player;

void characterInit(character *c){
	c->hookPos = c->gvel = c->vel =  vecZero();

	c->gliderFade = 0.f;
	c->animationIndex = c->animationTicksMax = c->animationTicksLeft = 0;

	c->flags         = 0;
	c->actionTimeout = 0;
	c->stepTimeout   = 0;
	c->activeItem    = 0;
	c->eMesh         = (void *)1234;

	c->blockMiningX  = c->blockMiningY = c->blockMiningZ = -1;
	c->hp = c->maxhp = 20;

	c->rot   = vecNew(135.f,0.f,0.f);
	c->gyoff = c->yoff = 0.f;
	c->pos   = vecAdd(worldGetSpawnPos(),vecNew(.5f,1.f,.5f));

	c->hook  = false;

	characterEmptyInventory(c);
}

void characterDie(character *c){
	characterInit(c);
}

int characterHitCheck(const vec pos, float mdd, int damage, int cause, u16 iteration, being source){
	int hits = 0;
	for(int i=0;i<32;i++){
		if(clients[i].state)               {continue;}
		if(clients[i].c == NULL)           {continue;}
		if(clients[i].c->temp == iteration){continue;}
		if(beingCharacter(i) == source)    {continue;}
		vec dis = vecSub(pos,clients[i].c->pos);
		if(vecDot(dis,dis) < mdd){
			msgBeingDamage(i,damage, cause, 1.f,beingCharacter(i),0,pos);
			msgBeingGotHit(  damage, cause, 1.f,beingCharacter(i),0);
			clients[i].c->temp = iteration;
			hits++;
		}
	}
	return hits;
}

bool characterDamage(character *c, int hp){
	return characterHP(c,-hp);
}

const char *characterGetName(const character *c) {
	(void)c;
	return "Unknown Player";
}
