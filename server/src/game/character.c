#include "character.h"

#include "../main.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/blockType.h"
#include "../game/itemDrop.h"
#include "../game/blockMining.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>

character characterList[128];
int characterCount = 0;
character *characterFirstFree = NULL;

void characterInit(character *c){
	int sx,sy,sz;

	c->gyoff  = 0.f;
	c->gvx = c->gvy = c->gvz = 0.f;

	c->flags = 0;
	c->animationIndex = c->animationTicksMax = c->animationTicksLeft = 0;

	c->actionTimeout = 0;
	c->stepTimeout   = 0;
	c->activeItem    = 0;

	c->blockMiningX  = c->blockMiningY = c->blockMiningZ = -1;
	c->hp = c->maxhp = 20;

	bigchungusGetSpawnPos(&world,&sx,&sy,&sz);
	c->yaw   = 135.f;
	c->pitch = 0.f;
	c->roll  = 0.f;
	c->yoff  = 0.f;
	c->x     = sx+0.5f;
	c->y     = sy+1.0f;
	c->z     = sz+0.5f;
	c->vx    = c->vy = c->vz = 0.f;
	c->hook  = false;
	c->hookx = c->hooky = c->hookz = 0.f;

	characterEmptyInventory(c);
}

character *characterNew(){
	character *c = NULL;
	if(characterFirstFree != NULL){
		c = characterFirstFree;
		characterFirstFree = c->nextFree;
	}
	if(c == NULL){
		if(characterCount >= (int)(sizeof(characterList) / sizeof(character))-1){
			fprintf(stderr,"characterList Overflow!\n");
			return NULL;
		}
		c = &characterList[characterCount++];
	}
	c->hook = NULL;
	characterInit(c);

	return c;
}

void characterFree(character *c){
	c->nextFree = characterFirstFree;
	characterFirstFree = c;
}

void characterDie(character *c){
	characterInit(c);
}
