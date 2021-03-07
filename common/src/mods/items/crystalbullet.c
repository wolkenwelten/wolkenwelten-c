static const int ITEMID=265;

#include "../api_v1.h"

int crystalbulletItemDropBurnUp(itemDrop *id){
	(void)ITEMID;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}
