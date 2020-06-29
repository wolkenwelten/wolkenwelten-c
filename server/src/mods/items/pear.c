static const int ITEMID=258;

#include "../../game/item.h"
#include "../../game/character.h"

void pearInit(){
	(void)ITEMID;
}

bool pearActivateItem(item *cItem,character *cChar){
	(void)cItem;

	if(cChar->hp == cChar->maxhp){return false;}
	if(itemDecStack(cItem,1)){
		characterHP(cChar,4);
		return true;
	}
	return false;
}
