static const int ITEMID=266;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"

void masterblasterInit(){
	(void)ITEMID;
}

mesh *masterblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool masterblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 120){return false;}
	beamblast(cChar,1,64);
	return true;
}
