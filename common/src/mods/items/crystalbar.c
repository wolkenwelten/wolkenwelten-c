static const int ITEMID=269;

#include "../api_v1.h"

void crystalbarInit(){
	recipeAdd2I(ITEMID,1, 4,8, 18,8); 
}

mesh *crystalbarGetMesh(item *cItem){
	(void)cItem;

	return meshCrystalbullet;
}

