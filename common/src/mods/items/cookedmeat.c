static const int ITEMID=279;

#include "../api_v1.h"

void cookedmeatInit(){
	lispDefineID("i-","meat cooked",ITEMID);
}

char *cookedmeatGetItemName(const item *cItem){
	(void)cItem;
	return "Cooked Meat";
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

int cookedmeatItemDropBurnUpCallback(itemDrop *id){
	id->itm.ID = I_Burntmeat;
	return 1;
}

int cookedmeatGetFireHealth(const itemDrop *id){
	(void)id;
	return 40;
}
