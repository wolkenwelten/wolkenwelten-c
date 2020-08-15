static const int ITEMID=257;

#include "../api_v1.h"

void bombInit(){
	recipeAdd1I(ITEMID,1, 256,3);
}

bool bombActivateItem(item *cItem,character *cChar, int to){
	(void)cItem;
	if(to < 200){return false;}
	if(itemDecStack(cItem,1)){
		grenadeNew(cChar,3);
		return true;
	}
	return false;
}

mesh *bombGetMesh(item *cItem){
	(void)cItem;

	return meshBomb;
}
