static const int ITEMID=271;

#include "../api_v1.h"

void crystalpickaxeInit(){
	recipeAdd2I(ITEMID,1, 17,4, 269,2);
}

int crystalpickaxeDamage(const item *cItem){
	(void)cItem;

	return 8;
}

int crystalpickaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 8;
	}else if(blockCat == DIRT){
		return 5;
	}
	return 1;
}

mesh *crystalpickaxeGetMesh(const item *cItem){
	(void)cItem;

	return meshCrystalpickaxe;
}

int crystalpickaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
