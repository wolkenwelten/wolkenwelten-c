static const int ITEMID=274;

#include "../api_v1.h"

void gliderInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,12), itemNew(I_Fur,8));
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,12), itemNew(I_Straw,16));
	lispDefineID("i-","glider",ITEMID);
}

char *gliderGetItemName(const item *cItem){
	(void)cItem;
	return "Glider";
}

mesh *gliderGetMesh(const item *cItem){
	(void)cItem;
	return meshGlider;
}

int gliderGetStackSize(const item *cItem){
	(void)cItem;
	return 1;
}
