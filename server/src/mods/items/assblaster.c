static const int ITEMID=266;

#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"


bool assblasterIsSingleItem(const item *cItem){
	return true;
}

bool assblasterHasMineAction(const item *cItem){
	return true;
}

bool assblasterMineAction(item *cItem, character *cChar, int to){
	if(to < 120){return false;}
	//beamblast(cChar->ent,1,64);
	return true;
}
