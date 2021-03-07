static const int ITEMID=259;

#include "../api_v1.h"

void stoneaxeInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
}

bool stoneaxePrimaryAction(item *cItem, character *cChar){
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.25f, *cItem, characterGetBeing(cChar), 3, THROWABLE_PITCH_SPIN | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}
