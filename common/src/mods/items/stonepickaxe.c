static const int ITEMID=260;

#include "../api_v1.h"

void stonepickaxeInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
	lispDefineID("i-","stone pickaxe",ITEMID);
}

char *stonepickaxeGetItemName(const item *cItem){
	(void)cItem;
	return "Stone Pickaxe";
}

int stonepickaxeDamage(const item *cItem){
	(void)cItem;

	return 4;
}

int stonepickaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 3;
	} else if(blockCat == DIRT){
		return 2;
	}
	return 1;
}

mesh *stonepickaxeGetMesh(const item *cItem){
	(void)cItem;

	return meshStonepickaxe;
}

int stonepickaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
