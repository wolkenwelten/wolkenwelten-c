static const int ITEMID=260;

#include "../../game/item.h"
#include "../../game/character.h"
#include "../../game/blockType.h"
#include "../../game/recipe.h"

void pickaxeInit(){
	recipeAdd2I(ITEMID,1, 17,2, 3,2);
}

int pickaxeBlockDamage(const item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == STONE){
		return 5;
	} else if(blockCat == DIRT){
		return 3;
	}
	return 1;
}

bool pickaxeIsSingleItem(const item *cItem){
	(void)cItem;
	return true;
}
