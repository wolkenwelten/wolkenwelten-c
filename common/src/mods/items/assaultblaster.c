static const int ITEMID=263;

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

bool assaultblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool assaultblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) < 1){return false;}
	characterDecItemAmount(cChar, 265, 1);
	characterAddCooldown(cChar,15);
	beamblast(cChar,1.f,0.2f,0.05f,6,1,8.f,1.f);
	return true;
}

bool assaultblasterActivateItem(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) <= 2){return false;}
	characterDecItemAmount(cChar, 265, 3);
	characterAddCooldown(cChar,64);
	beamblast(cChar,1.f,0.2f,0.05f,6,3,8.f,1.f);
	return true;
}

int assaultblasterGetAmmunition(item *cItem){
	(void)cItem;
	
	return 265;
}