static const int ITEMID=267;

#include "../api_v1.h"

void ironaxeInit(){
	recipeNew2(itemNew(ITEMID,1),itemNew(I_Stone_Axe,1),itemNew(I_Iron_Bar,4));
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
