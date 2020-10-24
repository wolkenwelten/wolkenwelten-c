static const int ITEMID=258;

#include "../api_v1.h"

bool pearSecondaryAction(item *cItem,character *cChar){
	if(characterGetHP(cChar) >= characterGetMaxHP(cChar)){return false;}
	if(characterTryToUse(cChar,cItem,200,1)){
		characterHP(cChar,8);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,4,600);
		sfxPlay(sfxChomp,1.f);
		return true;
	}
	return false;
}

mesh *pearGetMesh(const item *cItem){
	(void)cItem;

	return meshPear;
}

int pearGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}
