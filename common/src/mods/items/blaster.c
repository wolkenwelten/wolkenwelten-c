static const int ITEMID=261;

#include "../api_v1.h"

bool blasterPrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,80,3)){return false;}
	beamblast(cChar,1.f,8.0f,0.05f,3,1,2.f,1.f);
	return true;
}

int blasterItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}
