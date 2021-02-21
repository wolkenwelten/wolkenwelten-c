static const int ITEMID=271;

#include "../api_v1.h"

void crystalpickaxeInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Iron_Pick,1), itemNew(I_Crystal_Bar,4));
	lispDefineID("i-","crystal pickaxe",ITEMID);
}

int crystalpickaxeDamage(const item *cItem){
	(void)cItem;

	return 8;
}

int crystalpickaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 8;
	}else if(blockCat == DIRT){
		return 5;
	}
	return 1;
}

int crystalpickaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
