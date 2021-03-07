static const int ITEMID=278;

#include "../api_v1.h"

int meatItemDropCallback(const item *cItem, float x, float y, float z){
	(void)ITEMID;
	(void)cItem;
	(void)x;
	(void)y;
	(void)z;
	if(rngValM(1<<15) == 0){ return -1;}
	return 0;
}

int meatItemDropBurnUpCallback(itemDrop *id){
	id->itm.ID = I_Cookedmeat;
	return 1;
}
