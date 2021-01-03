#include "being.h"
#include "../game/animal.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/projectile.h"
#include "../game/water.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

beingList *beingListGet(u16 x, u16 y, u16 z){
	chunk *chnk = worldTryChunk(x,y,z);
	if(chnk != NULL){return &chnk->bl;}
	chungus *chng = worldTryChungus(x>>8,y>>8,z>>8);
	if(chng != NULL){return &chng->bl;}
	return NULL;
}

void beingSync(u8 c, being b){
	switch(beingType(b)){
	default:
		return;
	case BEING_ANIMAL:
		animalSync(c,beingID(b));
		return;
	case BEING_PROJECTILE:
		projectileSendUpdate(c,beingID(b));
		return;
	case BEING_ITEMDROP:
		itemDropUpdateMsg(c,beingID(b));
		return;
	case BEING_FIRE:
		fireSendUpdate(c,beingID(b));
		return;
	case BEING_WATER:
		waterSendUpdate(c,beingID(b));
		return;
	}
}

void beingListSync(u8 c, beingList *bl){
	for(beingListEntry *ble = bl->first; ble != NULL; ble = ble->next){
		for(uint i=0; i<countof(ble->v); i++){
			if(ble->v[i] == 0){break;}
			beingSync(c,ble->v[i]);
		}
	}
}
