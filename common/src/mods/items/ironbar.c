static const int ITEMID=266;

#include "../api_v1.h"

void ironbarInit(){
	lispDefineID("i-","iron bar",ITEMID);
}

char *ironbarGetItemName(const item *cItem){
	(void)cItem;
	return "Iron Bar";
}

mesh *ironbarGetMesh(const item *cItem){
	(void)cItem;
	(void)ITEMID;
	return meshIronbar;
}
