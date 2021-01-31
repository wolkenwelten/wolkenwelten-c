static const int ITEMID=274;

#include "../api_v1.h"

void gliderInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,16), itemNew(I_Fur,8));
	lispDefineID("i-","glider",ITEMID);
}

mesh *gliderGetMesh(const item *cItem){
	(void)cItem;
	return meshGlider;
}

int gliderGetStackSize(const item *cItem){
	(void)cItem;
	return 1;
}
