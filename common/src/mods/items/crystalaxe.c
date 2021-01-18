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
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.25f, *cItem, characterGetBeing(cChar), THROWABLE_PITCH_SPIN | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}

bool crystalaxeSecondaryAction(item *cItem, character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleThrowAim(cChar,2.f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
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
