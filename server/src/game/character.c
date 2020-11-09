#include "character.h"

#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>

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
	c->pos   = vecAdd(vecNewI(worldGetSpawnPos()),vecNew(.5f,1.f,.5f));

	c->hook  = false;

	characterEmptyInventory(c);
}

void characterDie(character *c){
	characterInit(c);
}

void characterDmgPacket(uint c, const packet *p){
	const i16 hp    = p->v.i16[0];
	const i16 cause = p->v.u16[1];

	const being target  = p->v.u32[1];
	const being culprit = beingCharacter(c);

	msgBeingGotHit(hp,cause,target,culprit);
	msgBeingDamage(beingID(target), hp, cause, target, culprit, clients[c].c->pos);
}
