static const int ITEMID=289;

#include "../api_v1.h"

bool stonespearPrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.35f, *cItem, characterGetBeing(cChar), 4, THROWABLE_TIP_HEAVY | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}
