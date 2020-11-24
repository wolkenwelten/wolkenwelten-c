static const int ITEMID=284;

#include "../api_v1.h"

void flamebulletInit(){
	recipeNew2(itemNew(ITEMID,10), itemNew(I_Crystal,1), itemNew(I_Coal,1));
}

mesh *flamebulletGetMesh(const item *cItem){
	(void)cItem;

	return meshFlamebullet;
}

int flamebulletGetStackSize(const item *cItem){
	(void)cItem;
	return 999;
}

int flamebulletItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}

int flamebulletGetFireDmg(const itemDrop *id){
	(void)id;
	return 6;
}

int flamebulletGetFireHealth(const itemDrop *id){
	(void)id;
	return 64;
}
