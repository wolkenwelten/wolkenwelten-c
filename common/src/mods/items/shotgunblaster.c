static const int ITEMID=264;
static const int MAGSIZE=60;

#include "../api_v1.h"

void shotgunblasterInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,12), itemNew(I_Iron_Bar,24), itemNew(I_Flamebullet,24));
	lispDefineID("i-","shotgun",ITEMID);
}

bool shotgunblasterPrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,128,6)){return false;}
	sfxPlay(sfxPhaser,0.5f);
	for(int i=0;i<64;i++){
		projectileNewC(cChar, 0, 4);
	}
	characterAddInaccuracy(cChar,32.f);
	characterAddRecoil(cChar,3.f);
	return true;
}

bool shotgunblasterSecondaryAction(item *cItem,character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleAim(cChar,1.5f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
	return false;
}

bool shotgunblasterTertiaryAction(item *cItem, character *cChar){
	return characterItemReload(cChar, cItem, 256);
}

float shotgunblasterGetInaccuracy(const item *cItem){
	(void)cItem;

	return 48.f;
}

int shotgunblasterGetAmmunition(const item *cItem){
	(void)cItem;

	return I_Flamebullet;
}

int shotgunblasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}

int shotgunblasterItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}

int shotgunblasterGetFireDmg(const itemDrop *id){
	(void)id;
	return 6;
}

int shotgunblasterGetFireHealth(const itemDrop *id){
	(void)id;
	return 64;
}
