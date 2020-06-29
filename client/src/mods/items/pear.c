static const int ITEMID=258;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
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

mesh *pearGetMesh(item *cItem){
	(void)cItem;

	return meshPear;
}
