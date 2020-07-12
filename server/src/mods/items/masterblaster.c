static const int ITEMID=266;

#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"

void masterblasterInit(){
	(void)ITEMID;
}

bool masterblasterIsSingleItem(const item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterHasMineAction(const item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;

	if(to < 120){return false;}
	return true;
}
