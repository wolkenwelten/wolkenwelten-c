static const int ITEMID=265;

#include "../api_v1.h"

void crystalbulletInit(){
	recipeAdd1I(ITEMID,8, 18,1); // Hematite Ore(1) -> Crystalbullet(8)
}

mesh *crystalbulletGetMesh(item *cItem){
	(void)cItem;

	return meshPear;
}

int crystalbulletGetStackSize(item *cItem){
	(void)cItem;
	
	return 999;
}