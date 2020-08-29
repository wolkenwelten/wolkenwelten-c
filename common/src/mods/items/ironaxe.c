static const int ITEMID=267;

#include "../api_v1.h"

void ironaxeInit(){
	recipeAdd2I(ITEMID,1, 17,2, 266,2);
}

int ironaxeBlockDamage(item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 5;
	}
	return 1;
}

mesh *ironaxeGetMesh(item *cItem){
	(void)cItem;

	return meshAxe;
}

int ironaxeGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}
