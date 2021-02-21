static const int ITEMID=283;
static const int MAGSIZE=90;

#include "../api_v1.h"

void flamethrowerInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,8), itemNew(I_Iron_Bar,8), itemNew(I_Crystalbullet,8));
	lispDefineID("i-","flamethrower",ITEMID);
}

int flamethrowerGetStackSize(const item *cItem){
	(void)cItem;
	return 1;
}

bool flamethrowerPrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,15,1)){return false;}
	sfxPlay(sfxPhaser,0.2f);
	for(uint i=0;i<4;i++){
		projectileNewC(cChar, 0, 5);
	}
	characterAddInaccuracy(cChar,7.f);
	characterAddRecoil(cChar,1.f);
	return true;
}

bool flamethrowerSecondaryAction(item *cItem, character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleAim(cChar,2.f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
	return false;
}

bool flamethrowerTertiaryAction(item *cItem, character *cChar){
	return characterItemReload(cChar, cItem, 50);
}

int flamethrowerGetAmmunition(const item *cItem){
	(void)cItem;
	return I_Flamebullet;
}

int flamethrowerGetMagSize(const item *cItem){
	(void)cItem;
	return MAGSIZE;
}

int flamethrowerItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}

int flamethrowerGetFireDmg(const itemDrop *id){
	(void)id;
	return 6;
}

int flamethrowerGetFireHealth(const itemDrop *id){
	(void)id;
	return 64;
}

float flamethrowerGetInaccuracy(const item *cItem){
	(void)cItem;

	return 16.f;
}
