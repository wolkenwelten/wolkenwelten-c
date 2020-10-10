static const int ITEMID=261;
static const int MAGSIZE=30;

#include "../api_v1.h"

void blasterInit(){
	(void)ITEMID;
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,4), itemNew(I_Iron_Bar,4), itemNew(I_Bullet,4));
}

mesh *blasterGetMesh(const item *cItem){
	(void)cItem;

	return meshBlaster;
}

int blasterGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}

bool blasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)to;

	if(!characterTryToShoot(cChar,cItem,80,3)){return false;}
	beamblast(cChar,1.2f,1.0f,0.15f,6,1,16.f,1.f);
	return true;
}

bool blasterSecondaryAction(item *cItem, character *cChar, int to){
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

int blasterGetAmmunition(const item *cItem){
	(void)cItem;

	return 265;
}

int blasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}

float blasterGetInaccuracy(const item *cItem){
	(void)cItem;

	return 8.f;
}
