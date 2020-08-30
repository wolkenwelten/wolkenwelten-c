static const int ITEMID=259;

#include "../api_v1.h"

void stoneaxeInit(){
	recipeAdd2I(ITEMID,1, 17,2, 3,2);
}

int stoneaxeDamage(item *cItem){
	(void)cItem;
	
	return 4;
}

int stoneaxeBlockDamage(item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 3;
	}
	return 1;
}

mesh *stoneaxeGetMesh(item *cItem){
	(void)cItem;

	return meshStoneaxe;
}

int stoneaxeGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}
