static const int ITEMID=271;

#include "../api_v1.h"

void crystalpickaxeInit(){
	recipeAdd2I(ITEMID,1, 17,4, 269,4);
}

int crystalpickaxeDamage(item *cItem){
	(void)cItem;
	
	return 8;
}

int crystalpickaxeBlockDamage(item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 8;
	}else if(blockCat == DIRT){
		return 5;
	}
	return 1;
}

mesh *crystalpickaxeGetMesh(item *cItem){
	(void)cItem;

	return meshCrystalpickaxe;
}

int crystalpickaxeGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}
