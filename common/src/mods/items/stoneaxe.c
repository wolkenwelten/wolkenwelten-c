static const int ITEMID=259;

#include "../api_v1.h"

void stoneaxeInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
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
