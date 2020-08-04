//static const int ITEMID=260;

#include "../../game/item.h"
#include "../../game/blockType.h"

int pickaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 5;
	} else if(blockCat == DIRT){
		return 3;
	}
	return 1;
}
