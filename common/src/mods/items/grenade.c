static const int ITEMID=256;

#include "../api_v1.h"

void grenadeInit(){
	recipeNew3(itemNew(ITEMID,3), itemNew(I_Iron_Bar,1), itemNew(I_Coal, 4), itemNew(I_Crystal, 1));
}

int grenadeItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 1*id->itm.amount, 0);
	return 0;
}
