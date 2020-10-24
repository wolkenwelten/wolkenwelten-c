static const int ITEMID=257;

#include "../api_v1.h"

void bombInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Iron_Bar,1), itemNew(I_Coal, 4), itemNew(I_Crystal, 1));
}

bool bombSecondaryAction(item *cItem,character *cChar){
	if(characterTryToUse(cChar,cItem,200,1)){
		grenadeNew(cChar,3,0,0);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,0,240);
		return true;
	}
	return false;
}

mesh *bombGetMesh(const item *cItem){
	(void)cItem;

	return meshBomb;
}

int bombGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}
