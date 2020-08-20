static const int ITEMID=263;
static const int MAGSIZE=60;

#include "../api_v1.h"

void assaultblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,24, 13,12); // Crystal(24) + Hematite Ore(12) -> Assaultblaster(1)
}

mesh *assaultblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

int assaultblasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool assaultblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) < 1){return false;}
	itemDecAmmo(cItem,1);
	characterAddCooldown(cChar,15);
	beamblast(cChar,1.f,0.2f,0.05f,6,1,8.f,1.f);
	return true;
}

bool assaultblasterSecondaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(itemGetAmmo(cItem) < 3){return false;}
	itemDecAmmo(cItem,3);
	characterAddCooldown(cChar,64);
	beamblast(cChar,1.f,0.2f,0.05f,6,3,8.f,1.f);
	return true;
}

bool assaultblasterTertiaryAction(item *cItem, character *cChar, int to){
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

int assaultblasterGetAmmunition(item *cItem){
	(void)cItem;

	return 265;
}

int assaultblasterGetMagSize(item *cItem){
	(void)cItem;

	return MAGSIZE;
}
