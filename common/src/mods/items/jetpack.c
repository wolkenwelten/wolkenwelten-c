static const int ITEMID=276;

#include "../api_v1.h"

void jetpackInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,16), itemNew(I_Coal,32));
	lispDefineID("i-","jetpack",ITEMID);
}

mesh *jetpackGetMesh(const item *cItem){
	(void)cItem;
	return meshGrenade;
}

int jetpackGetStackSize(const item *cItem){
	(void)cItem;
	return 1;
}
