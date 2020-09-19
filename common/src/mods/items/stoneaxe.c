static const int ITEMID=259;

#include "../api_v1.h"

void stoneaxeInit(){
	recipeAdd2I(ITEMID,1, 17,4, 3,2);
}

int stoneaxeDamage(const item *cItem){
	(void)cItem;

	return 4;
}

int stoneaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 3;
	}
	return 1;
}

mesh *stoneaxeGetMesh(const item *cItem){
	(void)cItem;

	return meshStoneaxe;
}

int stoneaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
