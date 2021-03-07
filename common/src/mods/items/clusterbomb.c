static const int ITEMID=273;

#include "../api_v1.h"

int clusterbombItemDropBurnUp(itemDrop *id){
	(void)ITEMID;
	(void)id;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 4, 0);
	return 0;
}
