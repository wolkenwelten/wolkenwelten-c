static const int ITEMID=259;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/blockType.h"
#include "../../game/recipe.h"
#include "../../game/item.h"
#include "../../game/character.h"

void axeInit(){
	recipeAdd2I(ITEMID,1, 17,2, 3,2);
}

int axeBlockDamage(item *cItem, blockCategory blockCat){
	(void)cItem;

	if(blockCat == WOOD){
		return 4;
	}
	return 1;
}

mesh *axeGetMesh(item *cItem){
	(void)cItem;

	return meshAxe;
}

bool axeIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}
