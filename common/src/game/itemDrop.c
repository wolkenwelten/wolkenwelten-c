#include "itemDrop.h"

#include "../game/character.h"
#include "../game/entity.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>

itemDrop itemDropList[1<<14];
uint     itemDropCount = 0;
int      itemDropFirstFree = -1;

itemDrop *itemDropGetByBeing(being b){
	if(beingType(b) != BEING_ITEMDROP){return NULL;}
	uint i = beingID(b);
	if(i >= itemDropCount){return NULL;}
	return &itemDropList[i];
}

being itemDropGetBeing(const itemDrop *id){
	if(id == NULL){return 0;}
	return beingItemDrop(id - itemDropList);
}
