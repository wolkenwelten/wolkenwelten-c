static const int ITEMID=265;

#include "../api_v1.h"

void crystalbulletInit(){
	recipeAdd1I(ITEMID,8, 18,1); // Hematite Ore(1) -> Crystalbullet(8)
}

mesh *crystalbulletGetMesh(const item *cItem){
	(void)cItem;

	return meshCrystalbullet;
}

int crystalbulletGetStackSize(const item *cItem){
	(void)cItem;

	return 999;
}
