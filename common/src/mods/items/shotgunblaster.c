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

bool shotgunblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)to;

	if(!characterTryToShoot(cChar,cItem,64,6)){return false;}
	beamblast(cChar,0.6f,2.f,0.04f,6,12,32.f,1.f);
	return true;
}

bool shotgunblasterSecondaryAction(item *cItem,character *cChar, int to){
	(void)to;

	if(!characterTryToShoot(cChar,cItem,64,6)){return false;}
	beamblast(cChar,0.4f,2.f,0.02f,6,20,32.f,4.f);
	return true;
}

bool shotgunblasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)to;
	return characterItemReload(cChar, cItem, 256);
}

float shotgunblasterGetInaccuracy(const item *cItem){
	(void)cItem;

	return 24.f;
}

int shotgunblasterGetAmmunition(const item *cItem){
	(void)cItem;

	return 265;
}

int shotgunblasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}
