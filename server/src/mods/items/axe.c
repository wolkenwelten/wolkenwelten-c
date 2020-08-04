//static const int ITEMID=259;

#include "../../game/blockType.h"
#include "../../game/item.h"

int axeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 4;
	}
	return 1;
}
