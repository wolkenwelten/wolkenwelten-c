static const int ITEMID=259;

#include "../../game/blockType.h"
#include "../../game/recipe.h"
#include "../../game/item.h"
#include "../../game/character.h"

void axeInit(){
	recipeAdd2I(ITEMID,1, 17,2, 3,2);
}

int axeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 4;
	}
	return 1;
}

bool axeIsSingleItem(const item *cItem){
	(void)cItem;

	return true;
}
