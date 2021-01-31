static const int ITEMID=289;

#include "../api_v1.h"

void stonespearInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
	lispDefineID("i-","stone spear",ITEMID);
}

int stonespearDamage(const item *cItem){
	(void)cItem;

	return 8;
}

int stonespearBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 1; // Destroy spear!
	}

	return 1;
}

bool stonespearPrimaryAction(item *cItem, character *cChar){
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.25f, *cItem, characterGetBeing(cChar), THROWABLE_PITCH_SPIN | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}

bool stonespearSecondaryAction(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}

mesh *stonespearGetMesh(const item *cItem){
	(void)cItem;

	return meshStoneaxe;
}

int stonespearGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}
