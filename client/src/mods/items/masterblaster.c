static const int ITEMID=262;

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

	
	if(to < 400){return false;}
	beamblast(cChar,2.f,8.f,4.f,512);
	return true;
}