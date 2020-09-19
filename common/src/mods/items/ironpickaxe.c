static const int ITEMID=268;

#include "../api_v1.h"

void ironpickaxeInit(){
	recipeAdd2I(ITEMID,1, 17,4, 266,2);
}

int ironpickaxeDamage(const item *cItem){
	(void)cItem;

	return 6;
}

int ironpickaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 5;
	} else if(blockCat == DIRT){
		return 3;
	}
	return 1;
}

mesh *ironpickaxeGetMesh(const item *cItem){
	(void)cItem;

	return meshIronpickaxe;
}

int ironpickaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
