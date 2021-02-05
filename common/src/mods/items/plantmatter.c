static const int ITEMID=292;

#include "../api_v1.h"

void plantmatterInit(){
	lispDefineID("i-","plantmatter",ITEMID);
}

mesh *plantmatterGetMesh(const item *cItem){
	(void)cItem;

	return meshPlantmatter;
}

int plantmatterItemDropCallback(const item *cItem, float x, float y, float z){
	(void)cItem;
	if(rngValA(65535) != 0){return 0;}
	item straw = itemNew(I_Straw,1);
	itemDropNewP(vecNew(x,y,z),&straw);
	return -1;
}

int plantmatterGetFireDmg(const itemDrop *id){
	(void)id;
	return 4;
}

int plantmatterGetFireHealth(const itemDrop *id){
	(void)id;
	return 240;
}
