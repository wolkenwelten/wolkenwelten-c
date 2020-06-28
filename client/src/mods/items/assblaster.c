static const int ITEMID=266;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"

mesh *assblasterGetMesh(item *cItem){
	return meshAssblaster;
}

bool assblasterIsSingleItem(item *cItem){
	return true;
}

bool assblasterHasMineAction(item *cItem){
	return true;
}

bool assblasterMineAction(item *cItem, character *cChar, int to){
	if(to < 120){return false;}
	beamblast(cChar,1,64);
	return true;
}
