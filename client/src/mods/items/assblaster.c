static const int ITEMID=266;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"

void assblasterInit(){
	(void)ITEMID;
}

mesh *assblasterGetMesh(item *cItem){
	(void)cItem;

	return meshAssblaster;
}

bool assblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool assblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool assblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 120){return false;}
	beamblast(cChar,1,64);
	return true;
}
