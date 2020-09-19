static const int ITEMID=266;

#include "../api_v1.h"

void ironbarInit(){
	recipeAdd2I(ITEMID,1, 4,4, 13,4);
}

mesh *ironbarGetMesh(const item *cItem){
	(void)cItem;

	return meshIronbar;
}
