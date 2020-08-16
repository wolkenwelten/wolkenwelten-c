static const int ITEMID=259;

#include "../api_v1.h"

void axeInit(){
	recipeAdd2I(ITEMID,1, 17,2, 3,2);
}

int axeBlockDamage(item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 4;
	}
	return 1;
}

mesh *axeGetMesh(item *cItem){
	(void)cItem;

	return meshAxe;
}

int axeGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}
