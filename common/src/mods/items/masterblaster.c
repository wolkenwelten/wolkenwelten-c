static const int ITEMID=262;

#include "../api_v1.h"

void masterblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,24, 13,12); // Crystal(24) + Hematite Ore(12) -> Masterblaster(1)
}

mesh *masterblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool masterblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,18) <= 0){return false;}
	characterDecItemAmount(cChar, 18, 1);
	characterAddCooldown(cChar,400);
	beamblast(cChar,3.f,8.f,2.f,1024,1,32.f,1.f);
	return true;
}

bool masterblasterActivateItem(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,18) <= 0){return false;}
	characterDecItemAmount(cChar, 18, 1);
	characterAddCooldown(cChar,20);
	beamblast(cChar,0.5f,0.1f,2.f,1024,1,32.f,1.f);
	return true;
}

int masterblasterGetAmmunition(item *cItem){
	(void)cItem;
	
	return 18;
}