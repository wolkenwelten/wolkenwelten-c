static const int ITEMID=273;

#include "../api_v1.h"

void clusterbombInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Iron_Bar,3), itemNew(I_Coal, 12), itemNew(I_Crystal, 4));
}

int clusterbombItemDropBurnUp(itemDrop *id){
	(void)id;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 4, 0);
	return 0;
}
