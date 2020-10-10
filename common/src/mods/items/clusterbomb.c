static const int ITEMID=273;

#include "../api_v1.h"

void clusterbombInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Iron_Bar,3), itemNew(I_Coal, 12), itemNew(I_Crystal, 4));
}

bool clusterbombSecondaryAction(item *cItem,character *cChar, int to){
	(void)cItem;
	if(to < 0){return false;}
	if(itemDecStack(cItem,1)){
		grenadeNew(cChar,1,48,1.f);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,0,300);
		return true;
	}
	return false;
}

mesh *clusterbombGetMesh(const item *cItem){
	(void)cItem;

	return meshBomb;
}

int clusterbombGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}
