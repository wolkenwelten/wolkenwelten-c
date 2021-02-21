static const int ITEMID=290;

#include "../api_v1.h"

void ironspearInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
	lispDefineID("i-","iron spear",ITEMID);
}

int ironspearDamage(const item *cItem){
	(void)cItem;

	return 8;
}

int ironspearBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 1; // Destroy spear!
	}

	return 1;
}

bool ironspearPrimaryAction(item *cItem, character *cChar){
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.5f, *cItem, characterGetBeing(cChar), 8, THROWABLE_TIP_HEAVY | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}

bool ironspearSecondaryAction(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}
