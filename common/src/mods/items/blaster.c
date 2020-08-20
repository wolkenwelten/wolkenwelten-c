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

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) < 3){return false;}
	itemDecAmmo(cItem,3);
	characterAddCooldown(cChar,80);
	beamblast(cChar,1.2f,1.0f,0.15f,6,1,16.f,1.f);
	return true;
}

bool blasterSecondaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) < 3){return false;}
	beamblast(cChar,1.2f,1.0f,0.05f,6,itemGetAmmo(cItem)*2,6.f,3.f);
	itemDecAmmo(cItem,itemGetAmmo(cItem));
	characterAddCooldown(cChar,200);
	return true;
}

bool blasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) == MAGSIZE){return false;}
	int ammoleft = characterGetItemAmount(cChar,265);
	if(ammoleft <= 0){return false;}
	ammoleft = MIN(MAGSIZE,ammoleft);
	characterDecItemAmount(cChar, 265, itemIncAmmo(cItem,ammoleft));
	characterAddCooldown(cChar,50);
	sfxPlay(sfxHookReturned,0.7f);
	return true;
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
