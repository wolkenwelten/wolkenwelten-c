static const int ITEMID=260;

#include "../api_v1.h"

void pickaxeInit(){
	recipeAdd2I(ITEMID,1, 17,2, 3,2);
}

int pickaxeBlockDamage(item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 5;
	} else if(blockCat == DIRT){
		return 3;
	}
	return 1;
}

mesh *pickaxeGetMesh(item *cItem){
	(void)cItem;

	return meshPickaxe;
}

int pickaxeGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}
