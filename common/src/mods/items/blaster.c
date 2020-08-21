static const int ITEMID=261;
static const int MAGSIZE=30;

#include "../api_v1.h"

void blasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,12, 13,6); // Crystal(12) + Hematite Ore(6) -> Blaster(1)
}

mesh *blasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

int blasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool blasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)to;

	if(!characterTryToShoot(cChar,cItem,80,3)){return false;}
	beamblast(cChar,1.2f,1.0f,0.15f,6,1,16.f,1.f);
	return true;
}

bool blasterSecondaryAction(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)to;

	if(!characterTryToShoot(cChar,cItem,200,3)){return false;}
	beamblast(cChar,1.2f,1.0f,0.05f,6,(itemGetAmmo(cItem)+3)*2,6.f,3.f);
	itemDecAmmo(cItem,itemGetAmmo(cItem));
	return true;
}

bool blasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)to;
	return characterItemReload(cChar, cItem, 50);
}

int blasterGetAmmunition(item *cItem){
	(void)cItem;

	return 265;
}

int blasterGetMagSize(item *cItem){
	(void)cItem;

	return MAGSIZE;
}

float blasterGetInaccuracy(item *cItem){
	(void)cItem;

	return 8.f;
}
