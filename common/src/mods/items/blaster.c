static const int ITEMID=261;

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

bool blasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool blasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) < 2){return false;}
	characterDecItemAmount(cChar, 265, 3);
	characterAddCooldown(cChar,100);
	beamblast(cChar,1.2f,1.0f,0.15f,6,1,16.f,1.f);
	return true;
}

bool blasterActivateItem(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) <= 0){return false;}
	characterDecItemAmount(cChar, 265, 1);
	characterAddCooldown(cChar,6);
	beamblast(cChar,0.8f,0.5f,0.05f,6,1,4.f,1.f);
	return true;
}

int blasterGetAmmunition(item *cItem){
	(void)cItem;
	
	return 265;
}