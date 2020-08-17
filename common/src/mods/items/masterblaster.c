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

int masterblasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool masterblasterHasPrimaryAction(item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) < 32){return false;}
	characterDecItemAmount(cChar, 265, 32);
	characterAddCooldown(cChar,350);
	beamblast(cChar,4.f,8.f,2.f,1024,1,32.f,1.f);
	return true;
}

bool masterblasterSecondaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) <= 0){return false;}
	characterDecItemAmount(cChar, 265, 4);
	characterAddCooldown(cChar,50);
	beamblast(cChar,3.f,0.1f,2.f,1,1,16.f,1.f);
	return true;
}

int masterblasterGetAmmunition(item *cItem){
	(void)cItem;
	
	return 265;
}