static const int ITEMID=265;

#include "../api_v1.h"

void crystalbulletInit(){
	recipeNew2(itemNew(ITEMID,10), itemNew(I_Crystal,1), itemNew(I_Coal,1));
}

mesh *crystalbulletGetMesh(const item *cItem){
	(void)cItem;

	return meshCrystalbullet;
}

int crystalbulletGetStackSize(const item *cItem){
	(void)cItem;

	return 999;
}
