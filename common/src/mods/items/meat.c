static const int ITEMID=278;

#include "../api_v1.h"

bool meatSecondaryAction(item *cItem,character *cChar){
	if(characterGetHP(cChar) >= characterGetMaxHP(cChar)){return false;}
	if(characterTryToUse(cChar,cItem,200,1)){
		characterHP(cChar,2);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,4,600);
		sfxPlay(sfxChomp,1.f);
		return true;
	}
	return false;
}

mesh *meatGetMesh(const item *cItem){
	(void)cItem;

	return meshMeat;
}

int meatGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}
