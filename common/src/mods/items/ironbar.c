static const int ITEMID=266;

#include "../api_v1.h"

void ironbarInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Hematite_Ore,4), itemNew(I_Coal,4));
}

mesh *ironbarGetMesh(const item *cItem){
	(void)cItem;

	return meshIronbar;
}
