static const int ITEMID=286;

#include "../api_v1.h"

int crystaldustItemDropBurnUpCallback(itemDrop *id){
	(void)ITEMID;
	id->itm.ID = I_Crystal_Bar;
	return 1;
}
