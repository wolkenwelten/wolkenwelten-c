static const int ITEMID=291;

#include "../api_v1.h"

void crystalspearInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
	lispDefineID("i-","crystal spear",ITEMID);
}

int crystalspearDamage(const item *cItem){
	(void)cItem;

	return 8;
}

int crystalspearBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 1; // Destroy spear!
	}

	return 1;
}

bool crystalspearPrimaryAction(item *cItem, character *cChar){
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.7f, *cItem, characterGetBeing(cChar), 12, THROWABLE_TIP_HEAVY | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}

bool crystalspearSecondaryAction(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}

mesh *crystalspearGetMesh(const item *cItem){
	(void)cItem;

	return meshCrystalspear;
}

int crystalspearGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
