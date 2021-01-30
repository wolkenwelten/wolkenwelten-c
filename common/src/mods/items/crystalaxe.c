static const int ITEMID=270;

#include "../api_v1.h"

void crystalaxeInit(){
	recipeNew2(itemNew(ITEMID,1),itemNew(I_Iron_Axe,1),itemNew(I_Crystal_Bar,4));
}

int crystalaxeDamage(const item *cItem){
	(void)cItem;

	return 8;
}

int crystalaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 8;
	}
	return 1;
}

bool crystalaxePrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.25, THROWABLE_PITCH_SPIN | THROWABLE_PIERCE)){return true;}
	return false;
}

bool crystalaxeSecondaryAction(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}

mesh *crystalaxeGetMesh(const item *cItem){
	(void)cItem;

	return meshCrystalaxe;
}

int crystalaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
