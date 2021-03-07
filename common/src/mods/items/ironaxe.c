static const int ITEMID=267;

#include "../api_v1.h"

bool ironaxePrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.25f, *cItem, characterGetBeing(cChar), 5, THROWABLE_PITCH_SPIN | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}
