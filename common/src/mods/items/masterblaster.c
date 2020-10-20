static const int ITEMID=262;
static const int MAGSIZE=90;

#include "../api_v1.h"

void masterblasterInit(){
	(void)ITEMID;
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,16), itemNew(I_Iron_Bar,12), itemNew(I_Bullet,16));
}

mesh *masterblasterGetMesh(const item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

int masterblasterGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}

bool masterblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)to;

	if(!characterTryToShoot(cChar,cItem,350,45)){return false;}
	beamblast(cChar,6.f,12.f,2.f,1024,1,32.f,1.f);
	return true;
}

bool masterblasterSecondaryAction(item *cItem, character *cChar, int to){
	(void)to;

	if(!characterTryToShoot(cChar,cItem,50,5)){return false;}
	beamblast(cChar,3.f,2.f,2.f,8,1,16.f,1.f);
	return true;
}

bool masterblasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)to;
	return characterItemReload(cChar, cItem, 200);
}

int masterblasterGetAmmunition(const item *cItem){
	(void)cItem;

	return 265;
}

int masterblasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}
