static const int ITEMID=266;

#include "../api_v1.h"

void ironbarInit(){
	recipeAdd2I(ITEMID,1, 4,1, 13,1); 
}

mesh *ironbarGetMesh(item *cItem){
	(void)cItem;

	return meshIronbar;
}

