static const int ITEMID=269;

#include "../api_v1.h"

void crystalbarInit(){
	recipeAdd2I(ITEMID,1, 4,4, 18,4);
}

mesh *crystalbarGetMesh(const item *cItem){
	(void)cItem;

	return meshCrystalbar;
}
