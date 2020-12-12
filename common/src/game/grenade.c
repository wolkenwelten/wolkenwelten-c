#include "../game/grenade.h"

#include "../game/animal.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../mods/api_v1.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>

grenade grenadeList[512];
uint    grenadeCount = 0;

grenade *grenadeGetByBeing(being b){
	if(beingType(b) != BEING_GRENADE){return NULL;}
	uint i = beingID(b);
	if(i >= grenadeCount){return NULL;}
	return &grenadeList[i];
}

being grenadeGetBeing(const grenade *g){
	if(g == NULL){return 0;}
	return beingGrenade(g - grenadeList);
}
