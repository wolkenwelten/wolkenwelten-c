static const int ITEMID=262;
static const int MAGSIZE=90;

#include "../api_v1.h"

void masterblasterInit(){
	(void)ITEMID;
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,16), itemNew(I_Iron_Bar,12), itemNew(I_Crystalbullet,16));
}

mesh *masterblasterGetMesh(const item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

int masterblasterGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}

bool masterblasterPrimaryAction(item *cItem, character *cChar){
	if(!characterTryToShoot(cChar,cItem,350,45)){return false;}
	beamblast(cChar,6.f,12.f,2.f,1024,1,32.f,1.f);
	return true;
}

bool masterblasterSecondaryAction(item *cItem, character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleAim(cChar,6.f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
	return false;
}

bool masterblasterTertiaryAction(item *cItem, character *cChar){
	return characterItemReload(cChar, cItem, 200);
}

int masterblasterGetAmmunition(const item *cItem){
	(void)cItem;

	return I_Crystalbullet;
}

int masterblasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}

int masterblasterItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}

int masterblasterGetFireDmg(const itemDrop *id){
	(void)id;
	return 6;
}

int masterblasterGetFireHealth(const itemDrop *id){
	(void)id;
	return 64;
}
