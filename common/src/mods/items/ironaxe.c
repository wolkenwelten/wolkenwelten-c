static const int ITEMID=267;

#include "../api_v1.h"

void ironaxeInit(){
	recipeAdd2I(ITEMID,1, 17,4, 266,2);
}

int ironaxeDamage(const item *cItem){
	(void)cItem;

	return 6;
}

int ironaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 5;
	}
	return 1;
}

mesh *ironaxeGetMesh(const item *cItem){
	(void)cItem;

	return meshIronaxe;
}

int ironaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
