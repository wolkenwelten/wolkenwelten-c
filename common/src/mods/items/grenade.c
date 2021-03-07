static const int ITEMID=256;

#include "../api_v1.h"

int grenadeItemDropBurnUp(itemDrop *id){
	(void)ITEMID;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 1*id->itm.amount, 0);
	return 0;
}
