static const int ITEMID=264;
static const int MAGSIZE=60;

#include "../api_v1.h"

void shotgunblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,36, 13,18); // Crystal(36) + Hematite Ore(18) -> Shotgunblaster(1)
}

mesh *shotgunblasterGetMesh(item *cItem){
	(void)cItem;

	return meshShotgunblaster;
}

int shotgunblasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool shotgunblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)to;

	if(!characterTryToShoot(cChar,cItem,64,6)){return false;}
	beamblast(cChar,0.6f,1.f,0.04f,6,12,32.f,1.f);
	return true;
}

bool shotgunblasterSecondaryAction(item *cItem,character *cChar, int to){
	(void)to;

	if(!characterTryToShoot(cChar,cItem,64,6)){return false;}
	beamblast(cChar,0.4f,1.f,0.02f,6,20,32.f,4.f);
	return true;
}

bool shotgunblasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)to;
	return characterItemReload(cChar, cItem, 256);
}

float shotgunblasterGetInaccuracy(item *cItem){
	(void)cItem;

	return 24.f;
}

int shotgunblasterGetAmmunition(item *cItem){
	(void)cItem;

	return 265;
}

int shotgunblasterGetMagSize(item *cItem){
	(void)cItem;

	return MAGSIZE;
}
