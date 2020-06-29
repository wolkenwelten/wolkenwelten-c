static const int ITEMID=257;

#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"
#include "../../game/recipe.h"

void bombInit(){
	recipeAdd1I(ITEMID,1, 256,3);
}

bool bombActivateItem(item *cItem,character *cChar){
	(void)cItem;
	(void)cChar;

	if(itemDecStack(cItem,1)){
		return true;
	}
	return false;
}
