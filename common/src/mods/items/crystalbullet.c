static const int ITEMID=265;

#include "../api_v1.h"

void crystalbulletInit(){
	recipeNew2(itemNew(ITEMID,10), itemNew(I_Crystal,1), itemNew(I_Coal,1));
	lispDefineID("i-","crystal bullet",ITEMID);
}

int crystalbulletItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}
