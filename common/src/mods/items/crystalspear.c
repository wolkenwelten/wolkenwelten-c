static const int ITEMID=291;

#include "../api_v1.h"

void crystalspearInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
}

bool crystalspearPrimaryAction(item *cItem, character *cChar){
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.7f, *cItem, characterGetBeing(cChar), 12, THROWABLE_TIP_HEAVY | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}
