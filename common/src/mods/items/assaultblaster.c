static const int ITEMID=263;
static const int MAGSIZE=60;

#include "../api_v1.h"

void assaultblasterInit(){
	(void)ITEMID;
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,8), itemNew(I_Iron_Bar,8), itemNew(I_Bullet,8));
}

mesh *assaultblasterGetMesh(const item *cItem){
	(void)cItem;

	return meshAssaultblaster;
}

int assaultblasterGetStackSize(const item *cItem){
	(void)cItem;

	return 1;
}

bool assaultblasterPrimaryAction(item *cItem, character *cChar){
	if(!characterTryToShoot(cChar,cItem,15,1)){return false;}
	sfxPlay(sfxPhaser,0.2f);
	projectileNewC(cChar, 0, 1);
	characterAddInaccuracy(cChar,7.f);
	return true;
}

bool assaultblasterSecondaryAction(item *cItem, character *cChar){
	if(!characterTryToShoot(cChar,cItem,64,3)){return false;}
	beamblast(cChar,.8f,0.2f,0.05f,2,3,8.f,1.f);
	return true;
}

bool assaultblasterTertiaryAction(item *cItem, character *cChar){
	return characterItemReload(cChar, cItem, 50);
}

int assaultblasterGetAmmunition(const item *cItem){
	(void)cItem;

	return 265;
}

int assaultblasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}
