static const int ITEMID=258;

#include "../api_v1.h"

void pearInit(){
	(void)ITEMID;
}

bool pearSecondaryAction(item *cItem,character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetHP(cChar) >= characterGetMaxHP(cChar)){return false;}
	if(itemDecStack(cItem,1)){
		characterHP(cChar,4);
		characterAddCooldown(cChar,100);
		return true;
	}
	return false;
}

mesh *pearGetMesh(item *cItem){
	(void)cItem;

	return meshPear;
}

int pearGetAmmunition(item *cItem){
	(void)cItem;
	
	return ITEMID;
}