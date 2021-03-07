static const int ITEMID=285;

#include "../api_v1.h"

int irondustItemDropBurnUpCallback(itemDrop *id){
	(void)ITEMID;
	id->itm.ID = I_Iron_Bar;
	return 1;
}
