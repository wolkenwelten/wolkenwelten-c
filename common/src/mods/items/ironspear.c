static const int ITEMID=290;

#include "../api_v1.h"

bool ironspearPrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.5f, *cItem, characterGetBeing(cChar), 8, THROWABLE_TIP_HEAVY | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}
