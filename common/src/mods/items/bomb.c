static const int ITEMID=257;

#include "../api_v1.h"

int bombItemDropBurnUp(itemDrop *id){
	(void)ITEMID;
	(void)id;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 3*id->itm.amount, 0);
	return 0;
}
