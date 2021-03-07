static const int ITEMID=285;

#include "../api_v1.h"

void irondustInit(){
	recipeNew1(itemNew(ITEMID,1), itemNew(I_Hematite_Ore,1));
}

int irondustItemDropBurnUpCallback(itemDrop *id){
	id->itm.ID = I_Iron_Bar;
	return 1;
}
