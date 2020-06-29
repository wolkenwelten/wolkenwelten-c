static const int ITEMID=266;

#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"

void assblasterInit(){
	(void)ITEMID;
}

bool assblasterIsSingleItem(const item *cItem){
	(void)cItem;

	return true;
}

bool assblasterHasMineAction(const item *cItem){
	(void)cItem;

	return true;
}

bool assblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;

	if(to < 120){return false;}
	return true;
}
