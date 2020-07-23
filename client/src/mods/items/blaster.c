static const int ITEMID=261;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"

void blasterInit(){
	(void)ITEMID;
}

mesh *blasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool blasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool blasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool blasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 100){return false;}
	beamblast(cChar,0.75f,1.f,0.25f,6);
	return true;
}