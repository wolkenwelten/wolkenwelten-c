static const int ITEMID=268;

#include "../api_v1.h"

void ironpickaxeInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Stone_Pick,1), itemNew(I_Iron_Bar,4));
	lispDefineID("i-","iron pickaxe",ITEMID);
}

char *ironpickaxeGetItemName(const item *cItem){
	(void)cItem;
	return "Iron Pickaxe";
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
