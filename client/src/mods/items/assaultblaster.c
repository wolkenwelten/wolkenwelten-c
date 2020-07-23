static const int ITEMID=263;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"

void assaultblasterInit(){
	(void)ITEMID;
}

mesh *assaultblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool assaultblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool assaultblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool assaultblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 10){return false;}
	beamblast(cChar,0.2f,0.2f,0.1f,1);
	return true;
}