static const int ITEMID=264;

#include "../api_v1.h"

void shotgunblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,36, 13,18); // Crystal(36) + Hematite Ore(18) -> Shotgunblaster(1)
}

mesh *shotgunblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool shotgunblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool shotgunblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool shotgunblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,18) <= 0){return false;}
	characterDecItemAmount(cChar, 18, 1);
	characterAddCooldown(cChar,256);
	beamblast(cChar,0.5f,1.f,0.04f,4,48,32.f,1.f);
	return true;
}

bool shotgunblasterActivateItem(item *cItem,character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,18) <= 0){return false;}
	characterDecItemAmount(cChar, 18, 1);
	characterAddCooldown(cChar,512);
	beamblast(cChar,0.5f,1.f,0.02f,4,128,32.f,4.f);
	return true;
}

float shotgunblasterGetInaccuracy(item *cItem){
	(void)cItem;

	return 24.f;
}

int shotgunblasterGetAmmunition(item *cItem){
	(void)cItem;
	
	return 18;
}