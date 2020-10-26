static const int ITEMID=279;

#include "../api_v1.h"

void cookedmeatInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Meat,1), itemNew(I_Oak,1));
}


bool cookedmeatSecondaryAction(item *cItem,character *cChar){
	if(characterGetHP(cChar) >= characterGetMaxHP(cChar)){return false;}
	if(characterTryToUse(cChar,cItem,200,1)){
		characterHP(cChar,20);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,4,600);
		sfxPlay(sfxChomp,1.f);
		return true;
	}
	return false;
}

mesh *cookedmeatGetMesh(const item *cItem){
	(void)cItem;

	return meshCookedmeat;
}

int cookedmeatGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}
