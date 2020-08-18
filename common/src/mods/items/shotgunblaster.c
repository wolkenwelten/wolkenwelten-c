static const int ITEMID=264;
static const int MAGSIZE=60;

#include "../api_v1.h"

void shotgunblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,36, 13,18); // Crystal(36) + Hematite Ore(18) -> Shotgunblaster(1)
}

mesh *shotgunblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

int shotgunblasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool shotgunblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) < 30){return false;}
	itemDecAmmo(cItem,30);
	characterAddCooldown(cChar,256);
	beamblast(cChar,1.f,1.f,0.04f,6,48,32.f,1.f);
	return true;
}

bool shotgunblasterSecondaryAction(item *cItem,character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) < 30){return false;}
	itemDecAmmo(cItem,30);
	characterAddCooldown(cChar,512);
	beamblast(cChar,1.f,1.f,0.02f,6,128,32.f,4.f);
	return true;
}

bool shotgunblasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) == MAGSIZE){return false;}
	int ammoleft = characterGetItemAmount(cChar,265);
	if(ammoleft <= 0){return false;}
	ammoleft = MIN(MAGSIZE,ammoleft);
	characterDecItemAmount(cChar, 265, itemIncAmmo(cItem,ammoleft));
	characterAddCooldown(cChar,50);
	return true;
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