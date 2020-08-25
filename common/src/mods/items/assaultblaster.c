static const int ITEMID=263;
static const int MAGSIZE=60;

#include "../api_v1.h"

void assaultblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,24, 13,12); // Crystal(24) + Hematite Ore(12) -> Assaultblaster(1)
}

mesh *assaultblasterGetMesh(item *cItem){
	(void)cItem;

	return meshAssaultblaster;
}

int assaultblasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool assaultblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)to;
	
	if(!characterTryToShoot(cChar,cItem,15,1)){return false;}
	beamblast(cChar,1.f,0.2f,0.05f,6,1,8.f,1.f);
	return true;
}

bool assaultblasterSecondaryAction(item *cItem, character *cChar, int to){
	(void)to;
	
	if(!characterTryToShoot(cChar,cItem,64,3)){return false;}
	beamblast(cChar,1.f,0.2f,0.05f,6,3,8.f,1.f);
	return true;
}

bool assaultblasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)to;
	return characterItemReload(cChar, cItem, 50);
}

int assaultblasterGetAmmunition(item *cItem){
	(void)cItem;

	return 265;
}

int assaultblasterGetMagSize(item *cItem){
	(void)cItem;

	return MAGSIZE;
}
