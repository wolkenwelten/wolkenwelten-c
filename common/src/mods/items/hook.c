static const int ITEMID=275;

#include "../api_v1.h"

void hookInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Iron_Bar,4), itemNew(I_Coal,4));
}

mesh *hookGetMesh(const item *cItem){
	(void)cItem;
	return meshBlaster;
}

int hookGetStackSize(const item *cItem){
	(void)cItem;
	return 1;
}