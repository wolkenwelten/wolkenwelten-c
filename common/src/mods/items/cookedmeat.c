static const int ITEMID=279;

#include "../api_v1.h"

int cookedmeatItemDropBurnUpCallback(itemDrop *id){
	(void)ITEMID;
	id->itm.ID = I_Burntmeat;
	return 1;
}
