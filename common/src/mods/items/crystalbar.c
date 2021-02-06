static const int ITEMID=269;

#include "../api_v1.h"

void crystalbarInit(){
	lispDefineID("i-","crystal bar",ITEMID);
}

char *crystalbarGetItemName(const item *cItem){
	(void)cItem;
	return "Crystal Bar";
}

mesh *crystalbarGetMesh(const item *cItem){
	(void)cItem;
	(void)ITEMID;
	return meshCrystalbar;
}
