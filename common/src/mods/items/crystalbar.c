static const int ITEMID=269;

#include "../api_v1.h"

void crystalbarInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Crystal,4), itemNew(I_Coal,4));
}

mesh *crystalbarGetMesh(const item *cItem){
	(void)cItem;

	return meshCrystalbar;
}
