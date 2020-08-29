static const int ITEMID=270;

#include "../api_v1.h"

void crystalaxeInit(){
	recipeAdd2I(ITEMID,1, 17,4, 269,4);
}

int crystalaxeBlockDamage(item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 8;
	}
	return 1;
}

mesh *crystalaxeGetMesh(item *cItem){
	(void)cItem;

	return meshAxe;
}

int crystalaxeGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}
