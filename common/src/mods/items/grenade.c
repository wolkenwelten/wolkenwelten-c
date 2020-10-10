static const int ITEMID=256;

#include "../api_v1.h"

void grenadeInit(){
	recipeNew3(itemNew(ITEMID,3), itemNew(I_Iron_Bar,1), itemNew(I_Coal, 4), itemNew(I_Crystal, 1));
}

bool grenadeSecondaryAction(item *cItem,character *cChar, int to){
	(void)cItem;
	if(to < 0){return false;}
	if(itemDecStack(cItem,1)){
		grenadeNew(cChar,1,0,0);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,0,300);
		return true;
	}
	return false;
}

mesh *grenadeGetMesh(const item *cItem){
	(void)cItem;

	return meshGrenade;
}

int grenadeGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}
