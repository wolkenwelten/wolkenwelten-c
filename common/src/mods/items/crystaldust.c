static const int ITEMID=286;

#include "../api_v1.h"

void crystaldustInit(){
	recipeNew1(itemNew(ITEMID,1), itemNew(I_Crystal,1));
}

int crystaldustItemDropBurnUpCallback(itemDrop *id){
	id->itm.ID = I_Crystal_Bar;
	return 1;
}
