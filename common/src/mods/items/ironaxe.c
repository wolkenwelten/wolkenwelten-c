static const int ITEMID=267;

#include "../api_v1.h"

void ironaxeInit(){
	recipeNew2(itemNew(ITEMID,1),itemNew(I_Stone_Axe,1),itemNew(I_Iron_Bar,4));
	lispDefineID("i-","iron axe",ITEMID);
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

bool ironaxePrimaryAction(item *cItem, character *cChar){
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.25f, *cItem, characterGetBeing(cChar), 5, THROWABLE_PITCH_SPIN | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}

bool ironaxeSecondaryAction(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}

mesh *ironaxeGetMesh(const item *cItem){
	(void)cItem;

	return meshIronaxe;
}

int ironaxeGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
