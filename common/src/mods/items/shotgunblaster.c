static const int ITEMID=264;
static const int MAGSIZE=60;

#include "../api_v1.h"

void shotgunblasterInit(){
	(void)ITEMID;
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,12), itemNew(I_Iron_Bar,24), itemNew(I_Bullet,24));
}

mesh *shotgunblasterGetMesh(const item *cItem){
	(void)cItem;

	return meshShotgunblaster;
}

int shotgunblasterGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}

bool shotgunblasterPrimaryAction(item *cItem, character *cChar){
	if(!characterTryToShoot(cChar,cItem,128,6)){return false;}
	sfxPlay(sfxPhaser,0.5f);
	for(int i=0;i<64;i++){
		projectileNewC(cChar, 0, 1);
	}
	characterAddInaccuracy(cChar,32.f);
	return true;
}

bool shotgunblasterSecondaryAction(item *cItem,character *cChar){
	if(!characterTryToShoot(cChar,cItem,256,6)){return false;}
	sfxPlay(sfxPhaser,0.7f);
	//characterAddInaccuracy(cChar,48.f);
	for(int i=0;i<96;i++){
		projectileNewC(cChar, 0, 1);
		characterAddInaccuracy(cChar,4.f);
	}
	return true;
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

	return 265;
}

int shotgunblasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}
